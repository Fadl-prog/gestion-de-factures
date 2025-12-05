#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/billing.h"

//Fontion pour creer une nouvelle facture et l'ajouter a la liste des factures
InvoiceNode* create_invoice(InvoiceNode** head , Student* student , int amount, char* due_date)
{   
    
    InvoiceNode* new_invoice = (InvoiceNode*)malloc(sizeof(InvoiceNode));//allocation dynamiqe du nouveau noeud
    
    //definir l'Id de la facture ajoutee de facon incrementative

    if(*head == NULL)
    {
        new_invoice->id = 0;//rendre l'id 0 si c'est la premiere facture
        *head = new_invoice; //rendre le nouveau noeud la tete de la liste
    }
    else{

        InvoiceNode* temp = *head;/*creer un temp commenceant a la tete de la liste puis 
        parcourant les noeuds jusqu'au dernier jusqu'a ce qu'il n'y'en a plus pour l'ajouter
         a la fin et lui donner un id incremente*/
        
         while (temp->next_invoice != NULL)
        {
            temp = temp->next_invoice;
        }

        new_invoice->id = temp->id + 1;
        temp->next_invoice = new_invoice;
    }

    //Lui passer l'id de l'etudiant associe dans student_id
    new_invoice->student_id = student->id;
    
    //meme chose pour amount et due_date et le reste des champs
    new_invoice->amount = amount;
    strcpy(new_invoice->due_date,due_date);
    new_invoice->next_invoice = NULL;
    strcpy(new_invoice->status,"unpaid"); //par default , la facture est non payee
    //ajouter le nouveau noeud a la fin de la liste
    return new_invoice;
}


//Fonction pour modifier une facture existante
void modify_invoice(InvoiceNode *head, int invoice_id)
{   
    //D'abord on  va verifier si la liste est vide
    if (head == NULL) {
        printf("Il n'y a pas de factures a modifier\n");
        return;
    }

    //On cherche la facture avec son id donne au prealable
    InvoiceNode *temp = head;//on commence par initialiser un temp a la tete de la liste
    while (temp && temp->id != invoice_id) {
        temp = temp->next_invoice;
    }
    //Arreter le temp quand il est NULL ou egal a la facture recherchee
    if (temp == NULL) {
        printf("La facture avec l'id %d n'existe pas\n", invoice_id);
        return;
    }

    //Afficher les options de modification , En troix choix
    int choice = 0;
    printf("Quelle information voulez-vous modifier ?\n"
           "1 - Montant\n"
           "2 - Date d'echeance\n"
           "3 - Statut\n> ");
    //si l'entree du choix n'est pas valide (pas entier par exemple),on affiche un message d'erreur et on quitte la fonction
    if(scanf("%d", &choice)!=1)
    {
        printf("Entrée invalide.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF) {} //bon ca cest chatgpt il m'a dit que ce que j'ai ecris c'est faux si je vide pas le buffer et il m'a donne cela 
        return;
    }
    //un switch case pour les choix et tout
    switch (choice) {
    //cas 1 , si choix cest 1 , on change amount
    case 1: {
        int new_amount;
        printf("Entrez le nouveau montant : ");
        //verifie toujours si scanf est valide avec un entier (on va la faire sur chaque choix et scanf)
        if (scanf("%d", &new_amount) == 1) {
            temp->amount = new_amount;
            printf("Montant mis a jour.\n");
        } else {
            printf("Montant invalide.\n");
        }
        break;
    }

    case 2: {
        char new_due_date[11]; // "dd-mm-yyyy" + '\0' 
        printf("Entrez la nouvelle date d'echeance (dd-mm-yyyy) : ");
        if (scanf("%10s", new_due_date) == 1) {
            strncpy(temp->due_date, new_due_date, sizeof(temp->due_date) - 1);
            temp->due_date[sizeof(temp->due_date) - 1] = '\0';
            printf("Date mise a jour.\n");
        } else {
            printf("Date invalide.\n");
        }
        break;
    }

    case 3: {
        char new_status[7]; // soi "paid" ,  "unpaid" ou "late"
        printf("Entrez le nouveau statut (paid, unpaid ou late) : ");
        scanf("%6s", new_status);
        if (strcmp(new_status,"paid") == 0 || strcmp(new_status,"unpaid") == 0 || strcmp(new_status,"late") == 0) 
        {
            strncpy(temp->status, new_status, sizeof(temp->status) - 1);
            temp->status[sizeof(temp->status)-1] = '\0';
            printf("statut mis a jour.\n");
        } else {
            printf("Statut invalide.\n");
        }
        break;
    }
    default:
        //si aucun de [1,2,3] n'a ete choisi , message d'erreur
        printf("Choix invalide.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF) {} //encore une fois vider le buffer si entree invalide
        break;
    }

}

//Fonction pour recupere le statut d'une facture
char* get_invoice_status(InvoiceNode* head , int invoice_id)
{
    if(head ==NULL)
    {
        return "Il n'y'a pas de factures dans la liste pour realiser cette operation\n";
    }
    InvoiceNode* temp = head ; //Initialiser le temp a la tete de la liste des factures 
    while(temp && temp->id != invoice_id)
    {
        temp  =temp->next_invoice; //Parcourir la liste jusqu'a trouver la facture avec l'id donne
    }
    if(temp == NULL)
    {
        return "La facture avec cet id n'existe pas\n";
    }
    return temp->status;

}

//Fonction pour supprimer une facture existante 

void delete_invoice(InvoiceNode** head , int invoice_id)
{
    //on verifie si la liste est vide
    if(*head == NULL)
    {
        printf("il n'y'a aucune facture a supprimer\n");
        return;
    }
    //on initialise un temp a la tete de la liste et un prev representant le noeud qui precede temp
    InvoiceNode* temp = *head;
    InvoiceNode* prev = NULL;
    //on s'arrete quand temp est NULL ou quand on trouve la facture recherchee
    while(temp && temp->id != invoice_id)
    {
        //prev devient temp et temp vance au noeud suivant
        prev = temp;
        temp = temp->next_invoice;
    }
    //si temp est NULL , la facture n'existe pas
    if(temp == NULL)
    {
        printf("La facture avec l'id %d n'existe pas\n", invoice_id);
        return;
    }
    //si prev est NULL , cela signifie que la facture a supprimer est la tete de la liste
    if(prev == NULL)
    {
        *head = temp->next_invoice; 
    }
    else{
        prev->next_invoice = temp->next_invoice;
    }
    free(temp); //liberer la memoire occupee par la facture supprimee
    printf("Facture avec l'id %d supprimee avec succes\n", invoice_id);
}

int main()
{
    //realiser une serie de tests pour verifier que les fonctions marchent bien
    InvoiceNode* invoice_list = NULL; //initialiser la liste des factures a NULL    
    Student student1 = { .id = 1, .name = "Alice", .classe = "10A", .next_student = NULL };
    Student student2 = { .id = 2, .name = "Bob", .classe = "10B", .next_student = NULL };
    //Creer des factures
    InvoiceNode* inv1 = create_invoice(&invoice_list, &student1, 500, "15-09-2024");
    InvoiceNode* inv2 = create_invoice(&invoice_list, &student2, 750, "20-09-2024");
    printf("Facture 1: ID=%d, StudentID=%d, Amount=%d, DueDate=%s, Status=%s\n", inv1->id, inv1->student_id, inv1->amount, inv1->due_date, inv1->status);
    printf("Facture 2: ID=%d, StudentID=%d, Amount=%d, DueDate=%s, Status=%s\n", inv2->id, inv2->student_id, inv2->amount, inv2->due_date, inv2->status);
    //Modifier une facture  
    modify_invoice(invoice_list, 0); //modifier la facture avec id 0
    //Obtenir le statut d'une facture   
    char* status = get_invoice_status(invoice_list, 0);
    printf("Statut de la facture 0: %s\n", status);
    //Supprimer une facture
    delete_invoice(&invoice_list, 1); //supprimer la facture avec id 1
    
    return 0;
}   