#ifndef REPORTS_H
#define REPORTS_H

#include "billing.h" 


// Structure pour stocker les informations détaillées d'une seule facture (max/min)
typedef struct {
    int invoice_id;
    int student_id;
    int amount;
} MaxMinInvoice;

// Structure pour le rapport général
typedef struct {
    int total_invoices;// Nombre total de factures
    int total_amount; // Montant total (somme de toutes les factures)
    float average_amount;// Montant moyen
    int count_paid;// Nombre de factures payées
    int count_unpaid;// Nombre de factures non payées
    int count_late;// Nombre de factures en retard
    MaxMinInvoice highest;// Facture la plus élevée
    MaxMinInvoice lowest;// Facture la plus basse
} GeneralReport;

// Structure pour le rapport étudiant
typedef struct {
    int total_invoices; // Total de factures pour cet étudiant
    int total_billed; // Montant total facturé
    int totl_paid; // Montant total payé
    int total_remaining;// Montant restant à payer
    InvoiceNode* student_invoices; // Liste chaînée des factures de cet étudiant (doit être libérée)
} StudentReport;



// Génère un rapport global sur toutes les factures.
BILLING_API GeneralReport generate_general_report(InvoiceNode* head);

// Génère un rapport détaillé pour un étudiant spécifique.
// Nécessite de libérer la liste student_invoices à l'intérieur de la structure.
BILLING_API StudentReport generate_student_report(InvoiceNode* head, int student_id);

// Génère un rapport mensuel.
BILLING_API GeneralReport generate_monthly_report(InvoiceNode* head, int month, int year);

// Génère un rapport annuel.
BILLING_API GeneralReport generate_yearly_report(InvoiceNode* head, int year);

#endif 