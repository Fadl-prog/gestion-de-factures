#ifndef REMINDER_H
#define REMINDER_H

#include "billing.h"//Inclure les structs etc presentes dans billing.h


//Fonction pour detecter une facture en retard

void get_today(int* day,int* month , int* year);//Fonction pour obtenir le jour , mois et annee d'aujourd'hui
void parse_due_date(char* due_date , int* day, int* month , int* year);//Fonction pour separer une date en jour , mois et annee
InvoiceNode* detect_late_invoice(InvoiceNode** head); //Fonction pour detecter les factures en retard
void alert_late_invoice(InvoiceNode* late_invoices); //Fonction pour alerter l'utilisateur 

#endif 