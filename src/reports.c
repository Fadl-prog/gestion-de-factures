#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include "../include/reports.h"


// Structure de date interne
typedef struct {
    int day;
    int month;
    int year;
} Date;

// Analyse la chaîne de date "dd-mm-yyyy"
static int parse_date(const char* due_date, int* day, int* month, int* year) {
    if (sscanf(due_date, "%d-%d-%d", day, month, year) != 3) {
        return 0;
    }
    return 1;
}

// Fonction utilitaire pour initialiser MaxMinInvoice
static void initialize_max_min(MaxMinInvoice *inv) {
    inv->invoice_id = -1;
    inv->student_id = -1;
    inv->amount = 0;
}


// Fonction interne pour remplir le rapport général à partir d'une sous-liste
static GeneralReport process_report(InvoiceNode* head) {
    GeneralReport report = {0};
    initialize_max_min(&report.highest);
    initialize_max_min(&report.lowest);
    
    // Initialiser le montant min/max pour les comparaisons futures
    int min_amount = INT_MAX; 
    int max_amount = INT_MIN;
    long total_amount_sum = 0; // Utiliser long pour éviter l'overflow
    
    InvoiceNode* current = head;
    
    while (current != NULL) {
        report.total_invoices++;
        total_amount_sum += current->amount;
        
        // Mettre à jour les compteurs de statut
        if (strcmp(current->status, "paid") == 0) {
            report.count_paid++;
        } else if (strcmp(current->status, "unpaid") == 0) {
            report.count_unpaid++;
        } else if (strcmp(current->status, "late") == 0) {
            report.count_late++;
        }
        
        // Trouver la facture la plus élevée (Highest)
        if (current->amount > max_amount) {
            max_amount = current->amount;
            report.highest.invoice_id = current->id;
            report.highest.student_id = current->student_id;
            report.highest.amount = current->amount;
        }

        // Trouver la facture la plus basse (Lowest)
        if (current->amount < min_amount || report.total_invoices == 1) {
            min_amount = current->amount;
            report.lowest.invoice_id = current->id;
            report.lowest.student_id = current->student_id;
            report.lowest.amount = current->amount;
        }
        
        current = current->next_invoice;
    }
    
    report.total_amount = (int)total_amount_sum;
    
    // Calculer la moyenne
    if (report.total_invoices > 0) {
        report.average_amount = (float)total_amount_sum / report.total_invoices;
    } else {
        report.average_amount = 0.0f;
    }

    return report;
}

// Fonction interne pour filtrer la liste des factures par mois/année
// Retourne une nouvelle liste chaînée (l'appelant doit la libérer)
static InvoiceNode* filter_by_month_year(InvoiceNode* head, int month, int year) {
    InvoiceNode* filtered_head = NULL;
    InvoiceNode* filtered_tail = NULL;
    InvoiceNode* current = head;

    while (current != NULL) {
        int due_day, due_month, due_year;
        
        if (parse_date(current->due_date, &due_day, &due_month, &due_year)) {
            // Filtrer par mois ET année (ou seulement par année si month est 0 pour le rapport annuel)
            if (due_year == year && (month == 0 || due_month == month)) {
                
                // Créer une COPIE du nœud
                InvoiceNode* new_node = (InvoiceNode*)malloc(sizeof(InvoiceNode));
                if (new_node == NULL) {
                    // En cas d'échec d'allocation, libérer la liste partielle et retourner NULL
                    InvoiceNode *temp;
                    while(filtered_head) {
                        temp = filtered_head;
                        filtered_head = filtered_head->next_invoice;
                        free(temp);
                    }
                    return NULL;
                }
                
                // Copier toutes les données du nœud original
                memcpy(new_node, current, sizeof(InvoiceNode));
                new_node->next_invoice = NULL;
                
                // Ajouter à la liste filtrée
                if (filtered_head == NULL) {
                    filtered_head = new_node;
                    filtered_tail = new_node;
                } else {
                    filtered_tail->next_invoice = new_node;
                    filtered_tail = new_node;
                }
            }
        }
        current = current->next_invoice;
    }
    return filtered_head;
}



// Génère un rapport global sur toutes les factures.
BILLING_API GeneralReport generate_general_report(InvoiceNode* head) {
    // La liste head est la liste complète, on la passe directement pour le traitement.
    return process_report(head);
}

// Génère un rapport détaillé pour un étudiant spécifique.
BILLING_API StudentReport generate_student_report(InvoiceNode* head, int student_id) {
    StudentReport report = {0};
    report.totl_paid = 0; // S'assurer que le champ totl_paid est initialisé à 0
    
    InvoiceNode* current = head;
    InvoiceNode* student_head = NULL;
    InvoiceNode* student_tail = NULL;

    while (current != NULL) {
        if (current->student_id == student_id) {
            report.total_invoices++;
            report.total_billed += current->amount;
            
            if (strcmp(current->status, "paid") == 0) {
                report.totl_paid += current->amount;
            }

            // Créer une COPIE du nœud pour la liste chaînée du rapport étudiant
            InvoiceNode* new_node = (InvoiceNode*)malloc(sizeof(InvoiceNode));
            if (new_node == NULL) {
                // Gestion d'erreur: libérer la liste partielle avant de retourner
                InvoiceNode *temp;
                while(student_head) {
                    temp = student_head;
                    student_head = student_head->next_invoice;
                    free(temp);
                }
                return (StudentReport){0}; // Retourner un rapport vide en cas d'échec
            }
            
            memcpy(new_node, current, sizeof(InvoiceNode));
            new_node->next_invoice = NULL;
            
            // Ajouter à la liste des factures de l'étudiant
            if (student_head == NULL) {
                student_head = new_node;
                student_tail = new_node;
            } else {
                student_tail->next_invoice = new_node;
                student_tail = new_node;
            }
        }
        current = current->next_invoice;
    }

    report.total_remaining = report.total_billed - report.totl_paid;
    report.student_invoices = student_head;
    
    return report;
}

// Génère un rapport mensuel.
BILLING_API GeneralReport generate_monthly_report(InvoiceNode* head, int month, int year) {
    // 1. Filtrer la liste pour ne garder que les factures du mois/année spécifiés
    InvoiceNode* filtered_list = filter_by_month_year(head, month, year);

    // 2. Traiter le rapport sur la liste filtrée
    GeneralReport report = process_report(filtered_list);
    
    // 3. IMPORTANT: Libérer la mémoire de la liste filtrée temporaire
    InvoiceNode* current = filtered_list;
    InvoiceNode* next;
    while (current != NULL) {
        next = current->next_invoice;
        free(current);
        current = next;
    }

    return report;
}

// Génère un rapport annuel.
BILLING_API GeneralReport generate_yearly_report(InvoiceNode* head, int year) {
    // Pour le rapport annuel, on utilise la même fonction de filtrage en passant 0 pour le mois.
    // L'implémentation de filter_by_month_year doit gérer le cas où month est 0 (regarder le code statique).
    
    // 1. Filtrer la liste pour ne garder que les factures de l'année spécifiée (month=0)
    InvoiceNode* filtered_list = filter_by_month_year(head, 0, year);

    // 2. Traiter le rapport sur la liste filtrée
    GeneralReport report = process_report(filtered_list);
    
    // 3. IMPORTANT: Libérer la mémoire de la liste filtrée temporaire
    InvoiceNode* current = filtered_list;
    InvoiceNode* next;
    while (current != NULL) {
        next = current->next_invoice;
        free(current);
        current = next;
    }

    return report;
}