import customtkinter as ctk
import tkinter as tk
from tkinter import messagebox, filedialog
import ctypes
import os
from datetime import datetime

# Importation pour la génération de PDF
from reportlab.lib.pagesizes import letter
from reportlab.pdfgen import canvas
from reportlab.lib import colors

# Importation du pont C
from billing_api import (
    billing_lib, 
    Student, 
    InvoiceNode, 
    GeneralReport, 
    list_c_to_python, 
    POINTER
)

# --- Variables Globales ---

# Pointeur vers la tête de la liste (initialisé à NULL au départ)
INVOICE_HEAD_PTR = POINTER(InvoiceNode)()
STUDENTS_HEAD_PTR = None  # pointeur vers la liste des étudiants
# Compteur pour les étudiants (juste pour la création, dans un vrai cas on lirait une table Student)
STUDENT_ID_COUNTER = 1 

class AppFacturation(ctk.CTk):
    def __init__(self):
        super().__init__()

        # Configuration de la fenêtre
        self.title("Gestion de Factures avec Base de Données (SQLite)")
        self.geometry("1100x700")
        ctk.set_appearance_mode("System")
        ctk.set_default_color_theme("blue")

        # Layout
        self.grid_columnconfigure(0, weight=1) 
        self.grid_columnconfigure(1, weight=3) 
        self.grid_rowconfigure(0, weight=1)

        # --- Cadre Gauche (Actions) ---
        self.frame_actions = ctk.CTkFrame(self)
        self.frame_actions.grid(row=0, column=0, padx=20, pady=20, sticky="nsew")
        self.frame_actions.grid_columnconfigure(0, weight=1)
        
        lbl_actions = ctk.CTkLabel(self.frame_actions, text="ACTIONS", font=ctk.CTkFont(size=20, weight="bold"))
        lbl_actions.pack(pady=(20, 10))

        self.btn_add = ctk.CTkButton(self.frame_actions, text="Ajouter Facture", command=self.open_add_invoice_dialog)
        self.btn_add.pack(pady=10, padx=20, fill="x")
        
        self.btn_refresh = ctk.CTkButton(self.frame_actions, text="Rafraîchir Liste", command=self.update_invoice_display)
        self.btn_refresh.pack(pady=10, padx=20, fill="x")

        # après btn_refresh
        self.btn_sort_date = ctk.CTkButton(self.frame_actions, text="Trier par Date", 
                                   command=self.sort_by_date)
        self.btn_sort_date.pack(pady=5, padx=20, fill="x")

        self.btn_sort_student = ctk.CTkButton(self.frame_actions, text="Trier par Étudiant", 
                                      command=self.sort_by_student)
        self.btn_sort_student.pack(pady=5, padx=20, fill="x")

        self.btn_detect_late = ctk.CTkButton(self.frame_actions, text="Détecter Retards", command=self.detect_and_show_late, fg_color="#D35B58", hover_color="#C72C41")
        self.btn_detect_late.pack(pady=10, padx=20, fill="x")

        ctk.CTkLabel(self.frame_actions, text="-----------------").pack(pady=5)

        self.btn_report = ctk.CTkButton(self.frame_actions, text="Voir Rapport Général", command=self.show_general_report)
        self.btn_report.pack(pady=10, padx=20, fill="x")

        self.btn_pdf = ctk.CTkButton(self.frame_actions, text="Exporter Rapport PDF", command=self.generate_pdf_report, fg_color="green", hover_color="darkgreen")
        self.btn_pdf.pack(pady=10, padx=20, fill="x")

        # --- Cadre Droite (Affichage) ---
        self.frame_display = ctk.CTkFrame(self)
        self.frame_display.grid(row=0, column=1, padx=20, pady=20, sticky="nsew")
        self.frame_display.grid_columnconfigure(0, weight=1)
        self.frame_display.grid_rowconfigure(1, weight=1)
        
        ctk.CTkLabel(self.frame_display, text="HISTORIQUE (Billing.db)", font=ctk.CTkFont(size=20, weight="bold")).grid(row=0, column=0, pady=10)

        self.invoice_text = ctk.CTkTextbox(self.frame_display, wrap="none", font=("Consolas", 14))
        self.invoice_text.grid(row=1, column=0, padx=15, pady=15, sticky="nsew")

        # --- INITIALISATION DE LA BASE DE DONNEES ---
        self.initialize_database()


    def initialize_database(self):
     """Initialise SQLite et charge les données."""
     global INVOICE_HEAD_PTR, STUDENTS_HEAD_PTR  # AJOUTE STUDENTS_HEAD_PTR 
    
     #  créer la table si elle n'existe pas (Appel C)
     success = billing_lib.init_db()
     if not success:
        messagebox.showerror("Erreur", "Impossible d'initialiser la base de données (init_db failed).")
        return

    #  charger les données depuis le fichier .db vers la liste C
     loaded_head = billing_lib.load_invoices_from_db()
    
    #  charger les étudiants depuis la DB (NOUVEAU)
     STUDENTS_HEAD_PTR = billing_lib.load_students_from_db()
     if STUDENTS_HEAD_PTR:
        print(f"Étudiants chargés depuis la DB")
     else:
        print("Aucun étudiant dans la DB ou chargement échoué")
    
    #  mettre à jour notre pointeur global Python
     if loaded_head:
        INVOICE_HEAD_PTR = loaded_head
        self.update_invoice_display()
     else:
        # si vide, on affiche juste un message vide
        self.update_invoice_display()


  
    def student_list_to_python(head_ptr):
     """Convertit liste chaînée C d'étudiants en liste Python."""
     students = []
     current = head_ptr
    
     while current:
        try:
            student = current.contents
        except ValueError:
            break
            
        students.append({
            'id': student.id,
            'name': student.name.decode('utf-8').strip('\x00'),
            'classe': student.classe.decode('utf-8').strip('\x00')
        })
        current = student.next_student
    
     return students



    def open_add_student_dialog(self):
     """Ajoute un nouvel étudiant."""
     global STUDENTS_HEAD_PTR
    
     dialog = ctk.CTkInputDialog(
        text="Format: NOM, CLASSE\nEx: Jean Dupont, Terminale A",
        title="Nouvel Étudiant"
    )
     input_data = dialog.get_input()
    
     if input_data:
        try:
            parts = input_data.split(',')
            if len(parts) != 2:
                raise ValueError("Format: Nom, Classe")
            
            name = parts[0].strip()
            classe = parts[1].strip()
            
            # Appel à la fonction C
            success = billing_lib.create_student_in_db(
                name.encode('utf-8'),
                classe.encode('utf-8')
            )
            
            if success:
                messagebox.showinfo("Succès", f"Étudiant {name} ajouté")
                # Recharger la liste des étudiants
                STUDENTS_HEAD_PTR = billing_lib.load_students_from_db()
            else:
                messagebox.showerror("Erreur", "Échec création étudiant")
                
        except Exception as e:
            messagebox.showerror("Erreur", f"Saisie invalide: {e}")





    def update_invoice_display(self):
        """Affiche la liste actuelle."""
        invoice_list_py = list_c_to_python(INVOICE_HEAD_PTR)
        
        self.invoice_text.configure(state="normal")
        self.invoice_text.delete("1.0", "end")

        if not invoice_list_py:
            self.invoice_text.insert("end", "\n   [Base de données vide ou aucun chargement.]\n")
            self.invoice_text.configure(state="disabled")
            return

        header_fmt = "{:<5} | {:<12} | {:<15} | {:<12} | {:<10}\n"
        separator = "-" * 75 + "\n"
        
        self.invoice_text.insert("end", header_fmt.format("ID", "Étudiant ID", "Montant (€)", "Échéance", "Statut"))
        self.invoice_text.insert("end", separator)

        for inv in invoice_list_py:
            line = header_fmt.format(
                inv['id'], inv['student_id'], inv['amount'], inv['due_date'], inv['status']
            )
            
            tag = "default"
            status = inv['status'].lower()
            if status == 'paid':
                tag = "paid_tag"
            elif status == 'late':
                tag = "late_tag"
            
            self.invoice_text.insert("end", line, tag)

        self.invoice_text.tag_config("paid_tag", foreground="#2CC985")
        self.invoice_text.tag_config("late_tag", foreground="#FF4D4D")
        self.invoice_text.configure(state="disabled")



    def open_add_invoice_dialog(self):
     """Ajoute une facture avec sélection d'étudiant."""
     global STUDENTS_HEAD_PTR, INVOICE_HEAD_PTR
    
     # 1. Vérifier s'il y a des étudiants
     students_list = AppFacturation.student_list_to_python(STUDENTS_HEAD_PTR)
    
     if not students_list:
        # Demander de créer un étudiant
        if messagebox.askyesno("Aucun étudiant", 
                              "Aucun étudiant dans la base.\nVoulez-vous en créer un ?"):
            self.open_add_student_dialog()
        return
    
     # 2. Boîte de dialogue SIMPLE
     dialog = ctk.CTkInputDialog(
        text="Format: ID_ÉTUDIANT, MONTANT, DATE\nEx: 1, 500, 31-12-2024\n\nÉtudiants disponibles:\n" + 
             "\n".join([f"ID {s['id']}: {s['name']} ({s['classe']})" for s in students_list]),
        title="Nouvelle Facture"
     )
    
     input_data = dialog.get_input()
    
     if input_data:
        try:
            parts = input_data.split(',')
            if len(parts) != 3:
                raise ValueError("Format: ID_Étudiant, Montant, Date")
            
            student_id = int(parts[0].strip())
            amount = int(parts[1].strip())
            due_date = parts[2].strip()
            
            # Chercher l'étudiant
            student_obj = None
            for s in students_list:
                if s['id'] == student_id:
                    student_obj = s
                    break
            
            if not student_obj:
                messagebox.showerror("Erreur", f"Étudiant ID {student_id} non trouvé")
                return
            
            if amount <= 0:
                messagebox.showerror("Erreur", "Montant > 0 requis")
                return
            
            # Créer la facture
            new_student_c = Student(
                id=student_id, 
                name=student_obj['name'].encode('utf-8'), 
                classe=student_obj['classe'].encode('utf-8')
            )
            
            result = billing_lib.create_invoice(
                ctypes.byref(INVOICE_HEAD_PTR), 
                ctypes.byref(new_student_c), 
                amount, 
                due_date.encode('utf-8')
            )
            
            if result:
                messagebox.showinfo("Succès", f"Facture créée pour {student_obj['name']}")
                self.update_invoice_display()
            else:
                messagebox.showerror("Erreur", "Échec création")
                
        except Exception as e:
            messagebox.showerror("Erreur", f"Saisie invalide: {e}")
    

    def sort_by_date(self):
      """Trie les factures par date."""
      global INVOICE_HEAD_PTR
    
      if not INVOICE_HEAD_PTR:
        messagebox.showinfo("Info", "Aucune facture à trier")
        return
    
      billing_lib.sort_ByDate(ctypes.byref(INVOICE_HEAD_PTR))
      self.update_invoice_display()
      messagebox.showinfo("Succès", "Factures triées par date")

    def sort_by_student(self):
      """Trie les factures par étudiant."""
      global INVOICE_HEAD_PTR
    
      if not INVOICE_HEAD_PTR:
        messagebox.showinfo("Info", "Aucune facture à trier")
        return
    
      billing_lib.sort_ByStudent(ctypes.byref(INVOICE_HEAD_PTR))
      self.update_invoice_display()
      messagebox.showinfo("Succès", "Factures triées par étudiant")



    def detect_and_show_late(self):
        """Détecte les retards."""
        
        late_list_ptr = billing_lib.detect_late_invoice(INVOICE_HEAD_PTR)
        self.update_invoice_display()

        late_invoices_py = list_c_to_python(late_list_ptr)
        count = len(late_invoices_py)
        
        if count > 0:
            msg = f"{count} facture(s) sont maintenant EN RETARD :\n\n"
            for inv in late_invoices_py:
                msg += f"• ID {inv['id']} - {inv['amount']}€\n"
            messagebox.showwarning("Alerte Retards", msg)
        else:
            messagebox.showinfo("Info", "Aucun nouveau retard.")



    def show_general_report(self):
        """Affiche le rapport."""
        report = billing_lib.generate_general_report(INVOICE_HEAD_PTR)

        if report.total_invoices == 0:
            messagebox.showinfo("Info", "Pas de données.")
            return

        msg = (
            f"--- RAPPORT GÉNÉRAL ---\n\n"
            f"Total Factures : {report.total_invoices}\n"
            f"Chiffre d'Affaires : {report.total_amount} €\n"
            f"Moyenne : {report.average_amount:.2f} €\n\n"
            f"Payées : {report.count_paid}\n"
            f"En Attente : {report.count_unpaid}\n"
            f"En Retard : {report.count_late}\n"
        )
        messagebox.showinfo("Rapport", msg)


    def generate_pdf_report(self):
        """Génère le PDF."""
        report = billing_lib.generate_general_report(INVOICE_HEAD_PTR)

        if report.total_invoices == 0:
            messagebox.showwarning("PDF", "Liste vide.")
            return

        file_path = filedialog.asksaveasfilename(
            defaultextension=".pdf",
            initialfile=f"Rapport_{datetime.now().strftime('%Y%m%d')}.pdf",
            filetypes=[("PDF files", "*.pdf")]
        )

        if not file_path:
            return

        try:
            c = canvas.Canvas(file_path, pagesize=letter)
            w, h = letter

            c.setFont("Helvetica-Bold", 18)
            c.drawString(50, h - 50, "Rapport Facturation (SQLite)")
            
            c.setFont("Helvetica", 10)
            c.drawString(50, h - 70, f"Généré le : {datetime.now().strftime('%d/%m/%Y %H:%M')}")
            c.line(50, h - 80, 550, h - 80)

            y = h - 120
            c.setFont("Helvetica", 12)
            
            lines = [
                (f"Total Factures : {report.total_invoices}", colors.black),
                (f"Montant Total : {report.total_amount} €", colors.black),
                (f"Payées : {report.count_paid}", colors.green),
                (f"Non Payées : {report.count_unpaid}", colors.red),
            ]

            for text, color in lines:
                c.setFillColor(color)
                c.drawString(50, y, text)
                y -= 25

            c.save()
            messagebox.showinfo("Succès", "PDF généré.")

        except Exception as e:
            messagebox.showerror("Erreur PDF", str(e))


if __name__ == "__main__":
 app = AppFacturation()
 app.mainloop()