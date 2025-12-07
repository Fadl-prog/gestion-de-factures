#ifndef  REPORTS_H
#define REPORTS_H
#include "billing.h"

// Fonction qui affichera un rapport global sur toutes les factures y compris le nb total de factures,
// montant total, montant moyen, facture la plus élevée + ses infos (par ex id étudiant + id facture),
// mm chose pour la facture la plus basse, nbres de factures : paid, unpaid et late
void general_report(InvoiceNode* head);
// fonction pour afficher un rapport par étudiant (contient nb total de factures, montant total, montant payé,
// restant, liste de toutes les factures + leur statut, facture la plus élevée, basse, etc)
void student_report(InvoiceNode* head, int student_id);
// rapport mensuel, contient presque les memes points que précédemment
void monthly_report(InvoiceNode* head, int month, int year);
// rapport annuel (idem)
void yearly_report(InvoiceNode* head, int year);

 #endif 
