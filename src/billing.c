#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "sqlite3.h"          
#include "../include/billing.h" 

#define DB_NAME "billing.db"

// Structure de date interne pour le tri 
typedef struct {
    int day;
    int month;
    int year;
} Date;

static Date parseDate(const char* s) {
    Date d = {0, 0, 0};
    sscanf(s, "%d-%d-%d", &d.day, &d.month, &d.year); 
    return d;
}

// Initialiser la base de données (Création de la table)
BILLING_API int init_db() {
    sqlite3 *db;
    char *err_msg = 0;
    
    // Ouvrir (ou créer) la base de données
    int rc = sqlite3_open(DB_NAME, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur ouverture DB: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    // Requête SQL pour créer la table si elle n'existe pas
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS Invoices ("
        "id INTEGER PRIMARY KEY,"  // On gère l'ID manuellement pour le sync avec la liste C
        "student_id INTEGER, "
        "amount INTEGER, "
        "due_date TEXT, "
        "status TEXT);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur SQL: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }

    sqlite3_close(db);
    return 1;
}

// Charger les factures depuis la DB vers la liste chaînée au démarrage
BILLING_API InvoiceNode* load_invoices_from_db() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    InvoiceNode *head = NULL;
    InvoiceNode *tail = NULL;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) return NULL;

    // Sélectionner toutes les factures triées par ID
    const char *sql = "SELECT id, student_id, amount, due_date, status FROM Invoices ORDER BY id ASC;";

    // Préparation de la requête
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        
        // Boucle sur chaque ligne de résultat
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            
            // Allocation d'un nouveau noeud
            InvoiceNode* new_node = (InvoiceNode*)malloc(sizeof(InvoiceNode));
            if (!new_node) break; // Sécurité mémoire

            // Remplissage des données depuis les colonnes SQL
            new_node->id = sqlite3_column_int(stmt, 0);
            new_node->student_id = sqlite3_column_int(stmt, 1);
            new_node->amount = sqlite3_column_int(stmt, 2);
            
            const unsigned char* date_text = sqlite3_column_text(stmt, 3);
            strncpy(new_node->due_date, (const char*)date_text, sizeof(new_node->due_date)-1);
            new_node->due_date[sizeof(new_node->due_date)-1] = '\0';

            const unsigned char* status_text = sqlite3_column_text(stmt, 4);
            strncpy(new_node->status, (const char*)status_text, sizeof(new_node->status)-1);
            new_node->status[sizeof(new_node->status)-1] = '\0';

            new_node->next_invoice = NULL;

            // Reconstruction de la liste chaînée
            if (head == NULL) {
                head = new_node;
                tail = new_node;
            } else {
                tail->next_invoice = new_node;
                tail = new_node;
            }
        }
    }
    
    sqlite3_finalize(stmt); // Nettoyage requête
    sqlite3_close(db);      // Fermeture DB
    return head;
}


// ==============================
// Fonctions Exportées (API) - Logique Métier + DB
// ==============================

BILLING_API InvoiceNode* create_invoice(InvoiceNode** head, Student* student, int amount, const char* due_date)
{ 
    if (!student || !due_date) return NULL;

    // --- 1. Gestion Mémoire (RAM) ---
    InvoiceNode* new_invoice = (InvoiceNode*)malloc(sizeof(InvoiceNode));
    if (new_invoice == NULL) return NULL;
    
    // Calcul de l'ID (Incrémentation basée sur la liste actuelle)
    if (*head == NULL) {
        new_invoice->id = 1; // Commence à 1
        *head = new_invoice;
    } else {
        InvoiceNode* temp = *head;
        while (temp->next_invoice != NULL) {
            temp = temp->next_invoice;
        }
        new_invoice->id = temp->id + 1;
        temp->next_invoice = new_invoice;
    }

    new_invoice->student_id = student->id;
    new_invoice->amount = amount;
    
    strncpy(new_invoice->due_date, due_date, sizeof(new_invoice->due_date) - 1);
    new_invoice->due_date[sizeof(new_invoice->due_date) - 1] = '\0';

    new_invoice->next_invoice = NULL;
    
    strncpy(new_invoice->status, "unpaid", sizeof(new_invoice->status) - 1);
    new_invoice->status[sizeof(new_invoice->status) - 1] = '\0';

    // --- 2. Gestion Base de Données (SQLite) ---
    sqlite3 *db;
    if (sqlite3_open(DB_NAME, &db) == SQLITE_OK) {
        char *sql = sqlite3_mprintf("INSERT INTO Invoices (id, student_id, amount, due_date, status) VALUES (%d, %d, %d, '%q', 'unpaid');", 
                                    new_invoice->id, student->id, amount, due_date);
        
        char *err_msg = 0;
        int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
        if (rc != SQLITE_OK) {
            // En cas d'erreur SQL, on l'affiche pour le debug (via stderr)
            fprintf(stderr, "SQL Error: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
        sqlite3_free(sql); // Important: libérer la chaîne créée par mprintf
        sqlite3_close(db);
    }

    return new_invoice;
}


BILLING_API int modify_invoice(InvoiceNode *head, int invoice_id, int new_amount, const char* new_due_date, const char* new_status)
{ 
    // --- 1. Gestion Mémoire (RAM) ---
    if (head == NULL) return 0;

    InvoiceNode *temp = head;
    while (temp != NULL && temp->id != invoice_id) {
        temp = temp->next_invoice;
    }
    
    if (temp == NULL) return 0; // Facture non trouvée

    // Mise à jour RAM
    if (new_amount >= 0) temp->amount = new_amount;
    
    if (new_due_date != NULL && strlen(new_due_date) > 0) {
        strncpy(temp->due_date, new_due_date, sizeof(temp->due_date) - 1);
        temp->due_date[sizeof(temp->due_date) - 1] = '\0';
    }

    if (new_status != NULL && strlen(new_status) > 0) {
        if (strcmp(new_status, "paid") == 0 || strcmp(new_status, "unpaid") == 0 || strcmp(new_status, "late") == 0) {
            strncpy(temp->status, new_status, sizeof(temp->status) - 1);
            temp->status[sizeof(temp->status) - 1] = '\0';
        } else {
            return -1; // Statut invalide
        }
    }

    // --- 2. Gestion Base de Données (SQLite) ---
    sqlite3 *db;
    if (sqlite3_open(DB_NAME, &db) == SQLITE_OK) {
        // Construction dynamique de la requête UPDATE
        // Note: Pour faire simple, on update tout si les valeurs sont valides
        
        char *sql = NULL;
        
        // Cas complexe : construire la requête selon ce qui change. 
        // Ici, pour simplifier le code C, on refait un UPDATE complet de la ligne avec les valeurs (déjà mises à jour dans RAM).
        sql = sqlite3_mprintf("UPDATE Invoices SET amount=%d, due_date='%q', status='%q' WHERE id=%d;", 
                              temp->amount, temp->due_date, temp->status, invoice_id);

        sqlite3_exec(db, sql, 0, 0, 0);
        sqlite3_free(sql);
        sqlite3_close(db);
    }

    return 1; 
}


BILLING_API int delete_invoice(InvoiceNode** head, int invoice_id)
{
    // --- 1. Gestion Mémoire (RAM) ---
    if (*head == NULL) return 0;

    InvoiceNode* temp = *head;
    InvoiceNode* prev = NULL;

    while (temp != NULL && temp->id != invoice_id) {
        prev = temp;
        temp = temp->next_invoice;
    }

    if (temp == NULL) return 0; // Non trouvé

    if (prev == NULL) {
        *head = temp->next_invoice;
    } else {
        prev->next_invoice = temp->next_invoice;
    }

    free(temp);

    // --- 2. Gestion Base de Données (SQLite) ---
    sqlite3 *db;
    if (sqlite3_open(DB_NAME, &db) == SQLITE_OK) {
        char *sql = sqlite3_mprintf("DELETE FROM Invoices WHERE id=%d;", invoice_id);
        sqlite3_exec(db, sql, 0, 0, 0);
        sqlite3_free(sql);
        sqlite3_close(db);
    }

    return 1;
}



BILLING_API char* get_invoice_status(InvoiceNode* head, int invoice_id)
{
    InvoiceNode* temp = head; 
    while (temp != NULL && temp->id != invoice_id) {
        temp = temp->next_invoice;
    }
    if (temp == NULL) return ""; 
    return temp->status;
}

BILLING_API int isPaid(const InvoiceNode* head) {
    const InvoiceNode* temp = head;
    while(temp != NULL) {
        if(strcmp(temp->status, "paid") == 0) return 1;
        temp = temp->next_invoice;
    }
    return 0;
}

BILLING_API int count_paid(InvoiceNode* head) {
    int count = 0;
    const InvoiceNode* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->status, "paid") == 0) count++;
        temp = temp->next_invoice;
    }
    return count;
}

BILLING_API int count_unpaid(InvoiceNode* head) {
    int count = 0;
    const InvoiceNode* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->status, "unpaid") == 0) count++;
        temp = temp->next_invoice;
    }
    return count;
}

BILLING_API int count_lateInv(InvoiceNode* head) {
    int count = 0;
    const InvoiceNode* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->status, "late") == 0) count++;
        temp = temp->next_invoice;
    }
    return count;
}


BILLING_API void sort_ByDate(InvoiceNode** head) {
    if(*head == NULL || (*head)->next_invoice == NULL) return;
    
    InvoiceNode* sort = NULL;      
    InvoiceNode* current = *head;   
    while(current != NULL) {
        InvoiceNode* next = current->next_invoice;
        Date d = parseDate(current->due_date);

        if(sort == NULL || 
           d.year < parseDate(sort->due_date).year || 
           (d.year == parseDate(sort->due_date).year && d.month < parseDate(sort->due_date).month) ||
           (d.year == parseDate(sort->due_date).year && d.month == parseDate(sort->due_date).month && d.day < parseDate(sort->due_date).day)) {
            current->next_invoice = sort;
            sort = current;
        } 
        else {
            InvoiceNode* temp = sort;
            InvoiceNode* prev = NULL;
            while(temp != NULL) {
                Date d_temp = parseDate(temp->due_date);
                if(d.year < d_temp.year || 
                   (d.year == d_temp.year && d.month < d_temp.month) ||
                   (d.year == d_temp.year && d.month == d_temp.month && d.day < d_temp.day)) {
                    break; 
                }
                prev = temp;
                temp = temp->next_invoice;
            }
            prev->next_invoice = current;
            current->next_invoice = temp;
        }
        current = next; 
    }
    *head = sort; 
}

BILLING_API void sort_ByStudent(InvoiceNode** head) {
    if(*head == NULL || (*head)->next_invoice == NULL) return;
    
    InvoiceNode* sort = NULL;      
    InvoiceNode* current = *head;  

    while(current != NULL) {
        InvoiceNode* next = current->next_invoice;
        if(sort == NULL) {
            current->next_invoice = NULL;
            sort = current;
        } 
        else {
            InvoiceNode* temp = sort;
            InvoiceNode* prev = NULL;
            while(temp != NULL && current->student_id > temp->student_id) {
                prev = temp;
                temp = temp->next_invoice;
            }
            if(prev == NULL) { 
                current->next_invoice = sort;
                sort = current;
            } else {
                prev->next_invoice = current;
                current->next_invoice = temp;
            }
        }
        current = next;
    }
    *head = sort; 
}

BILLING_API void print_invoice_list(InvoiceNode* head) {
    InvoiceNode* temp = head;
    while(temp != NULL) {
        printf("ID: %d, Amt: %d, Date: %s, Status: %s\n", 
               temp->id, temp->amount, temp->due_date, temp->status);
        temp = temp->next_invoice;
    }
}