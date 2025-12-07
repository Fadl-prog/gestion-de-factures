#ifndef BILLING_H
#define BILLING_H

//structure d'etudiant contenant tout ce qui le definit 
typedef struct Student
{
    int id ; //identifiant unique de l'etudiant
    char name[100]; //Nom de l'etudiant
    char classe[100];//Classe de l'etudiant
    struct Student *next_student; //l'etudiant le suivant dans l'ordre des id 

}Student;

//structure de facture contenant tout ce qui la definit en faisant en sorte qu'elle ait la structure d'une linked list
typedef struct InvoiceNode
{
    int id;//identifiant unique de la facture
    int student_id;//identifiant de l'etudiant associe a la facture
    int amount;//montant de la facture
    char due_date[11]; // 'dd-mm-yyyy'
    char status[7]; // soit 'paid' , 'unpaid' ou 'late'
    struct InvoiceNode* next_invoice ; //La facture la suivant dans l'ordre
}InvoiceNode;

//Fontion pour creer une nouvelle facture et l'ajouter a la liste des factures
InvoiceNode* create_invoice(InvoiceNode** head , Student* student ,int amount,char* due_date);
//Fonction pour modifier une facture existante
void modify_invoice(InvoiceNode *head, int invoice_id);
//Fonction pour supprimer une facture existante
void delete_invoice(InvoiceNode** head, int invoice_id);
//Fonction pour obtenir le statut d'une facture
char* get_invoice_status(InvoiceNode* head, int invoice_id);
/*Fonction qui vérifie si la liste des factures contient au moins une facture payée,
retournera 1 si c'est le cas, 0 sinon*/
int isPaid(InvoiceNode* head);
// Fonction pour compter les factures paid
int count_paid(InvoiceNode* head);
// Fonction pour compter les factures unpaid
int count_unpaid(InvoiceNode* head);
// Fonction pour compter les factures en retard
int count_lateInv(InvoiceNode* head);
// Trier les factures par date
void sort_ByDate(InvoiceNode** fact);
// Trier les factures par élève 
void sort_ByStudent(InvoiceNode** fact);
// affiche la liste de factures
void print_invoice_list(InvoiceNode* head);

#endif
