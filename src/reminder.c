#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/reminder.h" 

// Structure de date interne
typedef struct {
    int day;
    int month;
    int year;
} Date;



// Obtient la date du jour et la stocke.
int get_today(int* day, int* month, int* year) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t); 

    if (tm_info == NULL) {
        return 0;
    }

    *day = tm_info->tm_mday;
    *month = tm_info->tm_mon + 1;
    *year = tm_info->tm_year + 1900;
    
    return 1; 
}

// Analyse la chaîne de date "dd-mm-yyyy"
int parse_due_date(const char* due_date, int* day, int* month, int* year) {
    // Tente de lire 3 entiers séparés par des tirets
    if (sscanf(due_date, "%d-%d-%d", day, month, year) != 3) {
        return 0; // Format non respecté
    }
    return 1;
}

// Compare deux dates (d1 < d2 => -1, d1 > d2 => 1, d1 = d2 => 0)
static int compare_dates(Date d1, Date d2) {
    if (d1.year != d2.year) return d1.year < d2.year ? -1 : 1;
    if (d1.month != d2.month) return d1.month < d2.month ? -1 : 1;
    if (d1.day != d2.day) return d1.day < d2.day ? -1 : 1;
    return 0;
}


// Détecte les factures en retard et les retourne dans une nouvelle liste chaînée.
BILLING_API InvoiceNode* detect_late_invoice(InvoiceNode* head) {
    if (head == NULL) {
        return NULL;
    }
    
    // Récupération de la date d'aujourd'hui
    int current_day, current_month, current_year;
    if (!get_today(&current_day, &current_month, &current_year)) {
        return NULL; // Échec de récupération de la date
    }
    Date today = {current_day, current_month, current_year};
    
    InvoiceNode* late_invoices_head = NULL;
    InvoiceNode* late_invoices_tail = NULL;
    InvoiceNode* current = head;

    while (current != NULL) {
        int due_day, due_month, due_year;
        
        // On ne vérifie que si la facture n'est pas déjà payée (paid)
        if (strcmp(current->status, "paid") != 0 && parse_due_date(current->due_date, &due_day, &due_month, &due_year)) {
            Date due_date = {due_day, due_month, due_year};
            
            // Si la date d'échéance est passée (compare_dates < 0)
            if (compare_dates(due_date, today) < 0) {
                
                // Mettre à jour le statut dans la liste originale si nécessaire (unpaid -> late)
                if (strcmp(current->status, "unpaid") == 0) {
                    // Ici, nous modifions la liste principale (head)
                    strncpy(current->status, "late", sizeof(current->status) - 1);
                    current->status[sizeof(current->status) - 1] = '\0';
                }
                
                // Créer une COPIE du nœud de facture en retard pour la liste de retour
                InvoiceNode* new_late_node = (InvoiceNode*)malloc(sizeof(InvoiceNode));
                if (new_late_node == NULL) {
                    // En cas d'échec de malloc, le programme devrait idéalement s'arrêter ou libérer la mémoire.
                    // Pour une API, on retourne NULL pour indiquer l'échec.
                    return NULL;
                }
                
                // Copier les données de la facture (attention: ne pas copier le pointeur next_invoice de la liste principale)
                memcpy(new_late_node, current, sizeof(InvoiceNode));
                new_late_node->next_invoice = NULL; 
                
                // Ajouter le nœud copié à la nouvelle liste (ajout par la fin pour conserver l'ordre initial)
                if (late_invoices_head == NULL) {
                    late_invoices_head = new_late_node;
                    late_invoices_tail = new_late_node;
                } else {
                    late_invoices_tail->next_invoice = new_late_node;
                    late_invoices_tail = new_late_node;
                }
            }
        }
        current = current->next_invoice;
    }

    // Retourner la tête de la nouvelle liste des factures en retard (NULL si aucune)
    return late_invoices_head;
}