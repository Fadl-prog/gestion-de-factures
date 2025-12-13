#ifndef BILLING_H
#define BILLING_H


#ifdef _WIN32
    #ifdef BILLING_EXPORTS
        #define BILLING_API __declspec(dllexport)
    #else
        #define BILLING_API __declspec(dllimport)
    #endif
#else
    // Linux / Mac : aucun attribut requis
    #define BILLING_API
#endif



// Structure représentant un étudiant
typedef struct Student {
    int id;                     // Identifiant unique
    char name[100];             // Nom de l'étudiant
    char classe[100];           // Classe de l'étudiant
    struct Student* next_student; // Pointeur vers l'étudiant suivant
} Student;


// Structure représentant une facture
typedef struct InvoiceNode {
    int id;                     // Identifiant unique de la facture
    int student_id;             // Identifiant de l'étudiant associé
    int amount;                 // Montant
    char due_date[11];          // Format "dd-mm-yyyy"
    char status[7];             // paid / unpaid / late
    struct InvoiceNode* next_invoice; // Prochaine facture
} InvoiceNode;




// Créer une nouvelle facture
BILLING_API InvoiceNode* create_invoice(InvoiceNode** head, Student* student, int amount,const char* due_date);

// Modifier une facture
BILLING_API int modify_invoice(InvoiceNode *head, int invoice_id, int new_amount, const char* new_due_date, const char* new_status);

// Supprimer une facture
BILLING_API int delete_invoice(InvoiceNode** head, int invoice_id);

// Obtenir le statut d'une facture
BILLING_API char* get_invoice_status(InvoiceNode* head, int invoice_id);

// Vérifier si au moins une facture est payée
BILLING_API int isPaid(const InvoiceNode* head);

// Compter les factures payées
BILLING_API int count_paid(InvoiceNode* head);

// Compter les factures non payées
BILLING_API int count_unpaid(InvoiceNode* head);

// Compter les factures en retard
BILLING_API int count_lateInv(InvoiceNode* head);

// Trier par date
BILLING_API void sort_ByDate(InvoiceNode** fact);

// Trier par étudiant
BILLING_API void sort_ByStudent(InvoiceNode** fact);

// Afficher la liste des factures
BILLING_API void print_invoice_list(InvoiceNode* head);

// Initialiser la base de données (Crée la table si elle n'existe pas)
BILLING_API int init_db();

// Charger les factures depuis la DB vers la liste chaînée au démarrage
BILLING_API InvoiceNode* load_invoices_from_db();
#endif // BILLING_H
