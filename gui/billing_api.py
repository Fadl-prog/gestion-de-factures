import ctypes
from ctypes import c_int, c_float, c_char_p, POINTER, Structure, CDLL 

# chemin relatif 
import os
DLL_PATH = os.path.join(os.path.dirname(__file__), "billing_api.dll")

# --- 1. CHARGEMENT DE LA LIBRAIRIE C ---
try:
    billing_lib = CDLL(DLL_PATH)
except OSError as e:
    print(f"Erreur fatale : Impossible de charger la librairie C ({DLL_PATH}).")
    print("Vérifiez les dépendances MinGW (libgcc_s_dw2-1.dll, etc.)")
    raise


# --- 2. DÉFINITION DES STRUCTURES C EN PYTHON ---

class Student(Structure): pass 
class InvoiceNode(Structure): pass
class MaxMinInvoice(Structure): pass
class GeneralReport(Structure): pass
class StudentReport(Structure): pass

Student._fields_ = [
    ("id", c_int),
    ("name", ctypes.ARRAY(ctypes.c_char, 100)),
    ("classe", ctypes.ARRAY(ctypes.c_char, 100)),
    ("next_student", POINTER(Student))
]

InvoiceNode._fields_ = [
    ("id", c_int),
    ("student_id", c_int),
    ("amount", c_int),
    ("due_date", ctypes.ARRAY(ctypes.c_char, 11)),
    ("status", ctypes.ARRAY(ctypes.c_char, 7)),
    ("next_invoice", POINTER(InvoiceNode))
]

MaxMinInvoice._fields_ = [
    ("invoice_id", c_int),
    ("student_id", c_int),
    ("amount", c_int)
]

GeneralReport._fields_ = [
    ("total_invoices", c_int),
    ("total_amount", c_int),
    ("average_amount", c_float),
    ("count_paid", c_int),
    ("count_unpaid", c_int),
    ("count_late", c_int),
    ("highest", MaxMinInvoice),
    ("lowest", MaxMinInvoice)
]

StudentReport._fields_ = [
    ("total_invoices", c_int),
    ("total_billed", c_int),
    ("totl_paid", c_int),
    ("total_remaining", c_int),
    ("student_invoices", POINTER(InvoiceNode))
]


# --- 3. DÉFINITION DES PROTOTYPES DE FONCTIONS C ---

# --- Fonctions de BILLING ---
billing_lib.create_invoice.argtypes = [POINTER(POINTER(InvoiceNode)), POINTER(Student), c_int, c_char_p]
billing_lib.create_invoice.restype = POINTER(InvoiceNode)

billing_lib.modify_invoice.argtypes = [POINTER(InvoiceNode), c_int, c_int, c_char_p, c_char_p]
billing_lib.modify_invoice.restype = c_int

billing_lib.delete_invoice.argtypes = [POINTER(POINTER(InvoiceNode)), c_int]
billing_lib.delete_invoice.restype = c_int

# sort_ByDate
billing_lib.sort_ByDate.argtypes = [POINTER(POINTER(InvoiceNode))]
billing_lib.sort_ByDate.restype = None

# sort_ByStudent  
billing_lib.sort_ByStudent.argtypes = [POINTER(POINTER(InvoiceNode))]
billing_lib.sort_ByStudent.restype = None

# --- Fonctions DATABASE (SQLite) ---  <-- NOUVEAU ICI
billing_lib.init_db.argtypes = []
billing_lib.init_db.restype = c_int

billing_lib.load_invoices_from_db.argtypes = []
billing_lib.load_invoices_from_db.restype = POINTER(InvoiceNode)

# load_students_from_db
billing_lib.load_students_from_db.argtypes = []
billing_lib.load_students_from_db.restype = POINTER(Student)

# create_student_in_db
billing_lib.create_student_in_db.argtypes = [c_char_p, c_char_p]
billing_lib.create_student_in_db.restype = c_int

# get_student_by_id
billing_lib.get_student_by_id.argtypes = [POINTER(Student), c_int]
billing_lib.get_student_by_id.restype = POINTER(Student)

# --- Fonctions de REMINDER ---
billing_lib.detect_late_invoice.argtypes = [POINTER(InvoiceNode)]
billing_lib.detect_late_invoice.restype = POINTER(InvoiceNode)

# --- Fonctions de REPORTS ---
billing_lib.generate_general_report.argtypes = [POINTER(InvoiceNode)]
billing_lib.generate_general_report.restype = GeneralReport

billing_lib.generate_student_report.argtypes = [POINTER(InvoiceNode), c_int]
billing_lib.generate_student_report.restype = StudentReport



# --- 4. FONCTION UTILITAIRE PYTHON ---

def list_c_to_python(head_ptr):
    python_list = []
    current_ptr = head_ptr
    while current_ptr: 
        # Sécurité supplémentaire pour éviter les pointeurs invalides
        try:
            node = current_ptr.contents
        except ValueError:
            break # Pointeur invalide ou NULL
            
        data = {
            'id': node.id,
            'student_id': node.student_id,
            'amount': node.amount,
            'due_date': node.due_date.decode('utf-8').strip('\x00'), 
            'status': node.status.decode('utf-8').strip('\x00')
        }
        python_list.append(data)
        current_ptr = node.next_invoice
    return python_list