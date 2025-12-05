#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "../include/reminder.h"

//Fonction pour detecter une facture en retard

/*D'abord , pour definir si une facture est en retard , il faut pouvoir comparer la date actuelle d'aujourd'hui
Et celle de la due_date et voir si elle l'a depasse*/

//Tout d'abord  , on va recuperer la date d'aujourd'hui 

void get_today(int* day , int* month , int* year)
{
    time_t t = time(NULL); // ceci permet d'obtenir la date actuelle en seconde
    struct tm tm_info = *localtime(&t); // cette ligne c'est pour convertir en date (demande meme pas comment wallah je sais pas)
    *day = tm_info.tm_mday; // ca c'est jour du mois(1-31)
    *month = tm_info.tm_mon+1; //c'est de (0-11) du coup +1
    *year = tm_info.tm_year + 1900; //en fait tm_year ca donne depuis 1900 , du coup ca donne par exemple 2025->125 , faut ajouter 1900
}

//Maintenant , on va faire en sorte de parser le due_date , cad le separer en day , month , year numeriques

void parse_due_date(char* due_date,int* day ,int* month,int* year)
{
        char day_str[3],month_str[3], year_str[5];
        strncpy(day_str,due_date,2);day_str[2]='\0';
        strncpy(month_str,due_date+3,2);month_str[2]='\0';
        strncpy(year_str,due_date+6,4);
        year_str[4]='\0';
        *day = atoi(day_str);
        *month = atoi(month_str);
        *year = atoi(year_str);
}
//Maintenant qu'on peut comparer les dates , faisons place a la detection de factures defectueuses

InvoiceNode* detect_late_invoice(InvoiceNode** head)
{   
    InvoiceNode* late_invoices = NULL ; //initaliser une liste de factures en retard
    //si la liste est vide on ne fait rien
    if(*head == NULL)
    {
        printf("Il n'y'a aucune facture a verifier\n");
        return NULL;
    }
    InvoiceNode* temp = *head;//Initiliser un temp a la tete de la liste des factures
    //les valeurs auxquels on va assigner la date d'aujourd'hui
    int today_day , today_month , today_year;
    get_today(&today_day , &today_month , &today_year);//La fonction get_today qu'on a cree auparavant
    //Parcourir la liste de factures avec un pointeur temp
    while(temp)
    {
        int due_day , due_month , due_year;
        parse_due_date(temp->due_date , &due_day , &due_month , &due_year);
        //Comparer les dates et recuperer celles qui sont en retard pour les assigner a "late"
        if((due_year < today_year)||(due_year == today_year && due_month < today_month)|| (due_year == today_year && due_month == today_month && due_day < today_day))
        {
            //La facture est en retard
            //On va l'ajouter a la liste des factures en retard
            InvoiceNode* new_late_invoice = (InvoiceNode*)malloc(sizeof(InvoiceNode));
            //On va transferer term vers la liste de ceux en retard
            new_late_invoice->id = temp->id;
            new_late_invoice->student_id = temp->student_id;
            new_late_invoice->amount = temp->amount;
            strcpy(new_late_invoice->due_date,temp->due_date);
            //ici , on les met en "late" si elles ne le sont pas deja
            strcpy(new_late_invoice->status,"late");
            new_late_invoice->next_invoice = late_invoices;
            late_invoices = new_late_invoice;
        }
        temp = temp->next_invoice;
    }
    return late_invoices;
}
//Cette fonction va printer une alerte pour l'utilisateur pour ces factures en retard
void alert_late_invoice(InvoiceNode* late_invoices)
{
    //si late_invoices est NULL , c'est qu'il n'y'a aucune facture en retard
    if(late_invoices == NULL)
    {
        printf("il n'y'a aucune facture en retard\n");
        return;
    }
    //parcourir la liste et printer en consequence
    InvoiceNode* temp = late_invoices;
    while(temp)
    {
        printf("La facture avec l'id %d est en retard.A regler sous les plus brefs delais.\n",temp->id);
        temp = temp->next_invoice;
    }
}//Fonction test
int main()
{
    // --- Création de la liste chainée de factures ---
    InvoiceNode* head = NULL;

    // 1) Facture NON en retard (met une date future)
    InvoiceNode* f1 = malloc(sizeof(InvoiceNode));
    f1->id = 1;
    f1->student_id = 100;
    f1->amount = 500;
    strcpy(f1->due_date, "30/12/2030");
    strcpy(f1->status, "pending");
    f1->next_invoice = head;
    head = f1;

    // 2) Facture en retard
    InvoiceNode* f2 = malloc(sizeof(InvoiceNode));
    f2->id = 2;
    f2->student_id = 101;
    f2->amount = 700;
    strcpy(f2->due_date, "10/01/2023");
    strcpy(f2->status, "pending");
    f2->next_invoice = head;
    head = f2;

    // 3) Facture en retard
    InvoiceNode* f3 = malloc(sizeof(InvoiceNode));
    f3->id = 3;
    f3->student_id = 102;
    f3->amount = 400;
    strcpy(f3->due_date, "05/03/2024");
    strcpy(f3->status, "pending");
    f3->next_invoice = head;
    head = f3;

    // --- TEST : Détecter les factures en retard ---
    InvoiceNode* late_list = detect_late_invoice(&head);

    // --- TEST : Affichage ---
    printf("\n===== FACTURES EN RETARD =====\n\n");
    alert_late_invoice(late_list);

    return 0;
}