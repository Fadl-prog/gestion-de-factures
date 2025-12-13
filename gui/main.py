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
        global INVOICE_HEAD_PTR
        
        # 1. Créer la table si elle n'existe pas (Appel C)
        success = billing_lib.init_db()
        if not success:
            messagebox.showerror("Erreur", "Impossible d'initialiser la base de données (init_db failed).")
            return

        # 2. Charger les données depuis le fichier .db vers la liste C
        loaded_head = billing_lib.load_invoices_from_db()
        
        # 3. Mettre à jour notre pointeur global Python
        if loaded_head:
            INVOICE_HEAD_PTR = loaded_head
            self.update_invoice_display()
        else:
            # Si vide, on affiche juste un message vide
            self.update_invoice_display()


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
        """Ajoute une facture via C (qui l'écrira aussi en DB)."""
        dialog = ctk.CTkInputDialog(text="Format: MONTANT, DATE (Ex: 500, 31-12-2025)", title="Nouvelle Facture")
        input_data = dialog.get_input()
        
        if input_data:
            try:
                parts = input_data.split(',')
                if len(parts) != 2:
                    raise ValueError("Format attendu: Montant, Date")

                amount = int(parts[0].strip())
                date = parts[1].strip()
                
                # Pour l'exemple, on crée un étudiant générique
                global STUDENT_ID_COUNTER 
                new_student = Student(id=STUDENT_ID_COUNTER, name=b"Etudiant", classe=b"A1")
                STUDENT_ID_COUNTER += 1

                # Appel C : Crée le noeud RAM + INSERT INTO DB
                result = billing_lib.create_invoice(
                    ctypes.byref(INVOICE_HEAD_PTR), 
                    ctypes.byref(new_student), 
                    amount, 
                    date.encode('utf-8')
                )
                
                if result:
                    messagebox.showinfo("Succès", "Facture sauvegardée en base de données.")
                    self.update_invoice_display()
                else:
                    messagebox.showerror("Erreur C", "Erreur lors de la création.")
                
            except Exception as e:
                messagebox.showerror("Erreur", f"Saisie invalide: {e}")

    
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