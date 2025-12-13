#ifndef REMINDER_H
#define REMINDER_H


#include "billing.h" // Inclut les structures (InvoiceNode) et BILLING_API



// Obtient la date du jour 
int get_today(int* day, int* month, int* year);

// Analyse une date au format "dd-mm-yyyy" 
int parse_due_date(const char* due_date, int* day, int* month, int* year);




// Détecte les factures en retard et les retourne dans une nouvelle liste chaînée.
// Les statuts des factures en retard sont mis à jour (unpaid -> late) dans la liste originale.
// L'appelant doit libérer la mémoire de la liste retournée.
BILLING_API InvoiceNode* detect_late_invoice(InvoiceNode* head);

#endif // REMINDER_H