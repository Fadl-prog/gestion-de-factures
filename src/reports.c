#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/billing.h"
#include "../include/reminder.h"
#include "../include/reports.h"



// fonction general report
void general_report(InvoiceNode* head) {
    if(head == NULL) {
        printf("Aucun rapport a generer. Aucune facture detectee.\n");
        return;
    }
    InvoiceNode* temp = head;
    int nb_fact = 0;
    float total_fact = 0.0f; // f four indiquer que c un float et non un double
    float moyenne_fact = 0.0f;
    int total_paid, total_unpaid, total_late;

    // nb total de factures + montant total
    while (temp != NULL) {
        nb_fact++;
        total_fact += temp->amount;
        temp = temp->next_invoice;
    }
    // max et min
    InvoiceNode* maxFact = head;
    InvoiceNode* minFact = head;
    temp = head->next_invoice;
    while (temp != NULL) {
        if (temp->amount > maxFact->amount)
            maxFact = temp;
        if (temp->amount < minFact->amount)
            minFact = temp;
        temp = temp->next_invoice;
    }
    // moyenne
    moyenne_fact = total_fact / nb_fact;

    total_paid = count_paid(head);
    total_unpaid = count_unpaid(head);
    total_late = count_lateInv(head);

    printf("================RAPPORT GENERAL================\n");
    printf("Nombre total de facture : %d\n", nb_fact);
    printf("Montant total des factures : %.2f\n", total_fact);
    printf("Montant moyen (moyenne de toutes les factures) : %.2f\n", moyenne_fact);
    printf("Facture la plus elevee : %.2f  (ID Facture : %d, ID Etudiant : %d)\n", (float)maxFact->amount, maxFact->id, maxFact->student_id);
    printf("Facture la plus basse : %.2f  (ID Facture : %d, ID Etudiant : %d)\n", (float)minFact->amount, minFact->id, minFact->student_id);
    printf("Nombre de factures payées : %d\n", total_paid);
    printf("Nombre de factures impayées : %d\n", total_unpaid);
    printf("Nombre de factures en retard : %d\n", total_late);
}


// fonction pour afficher rapport par étudiant
void student_report(InvoiceNode* head, int student_id) {
    if (head == NULL) {
        printf("Aucun rapport a generer. Aucune facture detectee.\n");
        return;
    }

    InvoiceNode* temp = head;

    int nb_fact_etu = 0;
    float total_fact_etu = 0.0f;
    float moyenne_fact_etu = 0.0f;

    // pointeur vers la facture maximale de l'étudiant
    InvoiceNode* maxFactEtu = NULL;

    while (temp != NULL) {
        if (temp->student_id == student_id) {
            nb_fact_etu++;
            total_fact_etu += (float)temp->amount;
            // mise à jour du maximum
            if (maxFactEtu == NULL || temp->amount > maxFactEtu->amount) {
                maxFactEtu = temp;
            }
        }
        temp = temp->next_invoice;
    }
    if (nb_fact_etu == 0) {
        printf("Aucune facture trouvée pour l'étudiant %d.\n", student_id);
        return;
    }
    moyenne_fact_etu = total_fact_etu / nb_fact_etu;
    // affichage du rapport
    printf("=========== RAPPORT DE L'ETUDIANT %d ==========\n", student_id);
    printf("Nombre total de factures : %d\n", nb_fact_etu);
    printf("Montant total : %.2f\n", total_fact_etu);
    printf("Moyenne : %.2f\n", moyenne_fact_etu);

    if (maxFactEtu != NULL) {
        printf("Facture maximale : %.2f MAD (ID facture %d)\n", maxFactEtu->amount, maxFactEtu->id);
    }

    temp = head;
    printf("\n Liste des factures de l'etudiant %d :\n", student_id);
    while(temp != NULL) {
        if(temp->student_id == student_id) {
            printf("- Facture %d | %11s | %.2f MAD | %s\n", temp->id, temp->due_date, (float)temp->amount, temp->status);
        }
        temp = temp->next_invoice;
    }
    
}

// Rapport mensuel 
void monthly_report(InvoiceNode* head, int month, int year) {
    if(head == NULL) {
        printf("Aucun rapport a generer. Aucune facture detectee.\n");
        return;
    }

    InvoiceNode* temp = head;
    int nb_fact_mois = 0;
    float total_fact_mois = 0.0f;
    InvoiceNode* maxFactMois = NULL;
    int d, m, y; 

    while (temp != NULL) {
      // on extrait dd-mm-yyyy avec la fonction parse_due_date (depuis le header reminder.h)
        parse_due_date(temp->due_date, &d, &m, &y);
        if (m == month && y == year) {
            nb_fact_mois++;
            total_fact_mois += (float)temp->amount;
            if (maxFactMois == NULL || temp->amount > maxFactMois->amount) {
                maxFactMois = temp;
            }
        }
        temp = temp->next_invoice;
    }
    if (nb_fact_mois == 0) {
        printf("Aucune facture trouvee pour %02d/%d.\n", month, year);
        return;
    }

    float moyenne_fact_mois = total_fact_mois / nb_fact_mois;

    // affichage
    printf("\n===== Rapport Mensuel : %02d/%d =====\n", month, year);
    printf("Nombre total de factures : %d\n", nb_fact_mois);
    printf("Montant total : %.2f MAD\n", total_fact_mois);
    printf("Moyenne des factures : %.2f MAD\n", moyenne_fact_mois);

    if (maxFactMois != NULL) {
        printf("Facture maximale : %.2f MAD (ID : %d, Etudiant : %d, Date : %s, Statut : %s)\n",
               (float)maxFactMois->amount, maxFactMois->id, maxFactMois->student_id,
               maxFactMois->due_date, maxFactMois->status);
    }

    printf("\nListe des factures du mois :\n");
    temp = head;
    while (temp != NULL) {
        parse_due_date(temp->due_date, &d, &m, &y);
        if (m == month && y == year) {
            printf(" - Facture : %d | Etudiant : %d | %02d-%02d-%04d | %.2f MAD | %s\n",
                   temp->id, temp->student_id,
                   d, m, y,
                   (float)temp->amount,
                   temp->status);
        }
        temp = temp->next_invoice;
    }
}

// rapport annuel 
void yearly_report(InvoiceNode* head, int year) {
    if(head == NULL) {
        printf("Aucun rapport a generer. Aucune facture detectee.\n");
        return;
    }
    InvoiceNode* temp = head;
    int nb_fact_an = 0;
    float total_fact_an = 0.0f;
    int d, m, y;
    InvoiceNode* maxFactAn = NULL;
    while(temp != NULL) {
        parse_due_date(temp->due_date,&d,&m,&y);
        if(y == year) {
            nb_fact_an++;
            total_fact_an += (float)temp->amount;
            if(maxFactAn == NULL || temp->amount > maxFactAn->amount){
            maxFactAn = temp;
            }
        }
        temp = temp->next_invoice;
    }

    if(nb_fact_an == 0) {
        printf("Il n'y a aucune facture enregistree en %d.\n", year);
        return;
    }

    float moyenne_fact_an = total_fact_an / nb_fact_an;
    // affichage
    printf("\n========= RAPPORT ANNUEL : ANNEE %d =========\n", year);
    printf("Nombre total de factures : %d\n", nb_fact_an);
    printf("Montant total : %.2f MAD\n", total_fact_an);
    printf("Moyenne des factures : %.2f MAD\n", moyenne_fact_an);

    if(maxFactAn != NULL) {
        printf("Facture maximale : %.2f MAD (ID: %d, Etudiant: %d, Date: %s, Statut: %s)\n",
        (float)maxFactAn->amount, maxFactAn->id, maxFactAn->student_id, maxFactAn->due_date, maxFactAn->status);
    }
    
    temp = head;
    printf("\nListe des factures enregistrees en %d\n", year);
    while(temp !=  NULL){
        parse_due_date(temp->due_date,&d,&m,&y);
        if(y == year) {
            printf(" - Facture %d | Etudiant %d | %02d-%02d-%04d | %.2f MAD | %s\n",
                   temp->id, temp->student_id,
                   d, m, y,
                   (float)temp->amount,
                   temp->status);
        }
       temp = temp->next_invoice;
    }
    
}
    


