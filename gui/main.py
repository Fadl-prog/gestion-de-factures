from tkinter import *
import customtkinter as ctk

#Initialisation de la fenetre principale 
root = ctk.CTk()
ctk.set_appearance_mode("Dark")#theme sur dark 
ctk.set_default_color_theme("dark-blue")#theme bleu fonce

root.title("Gestion de Factures scolaire")#definir le titre de la fenetre
root.geometry("800x600")#sa geometrie(taille de base)
root.state('zoomed')#ouvrir en plein ecran

side_frame = ctk.CTkFrame(root,height=100,width=25,fg_color="#1f538d")#La frame ou se trouve le message de bienvenue
side_frame.pack(fill=Y,side=LEFT)

welcome_label = ctk.CTkLabel(master = side_frame , text="Bienvenue",font = ("Arial",20,"bold"),fg_color="#1f538d",text_color="white")#Le message de bienvenue 
welcome_label.pack(anchor= N)

root.mainloop()
