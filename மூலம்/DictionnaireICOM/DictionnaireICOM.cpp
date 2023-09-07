/**
 * Copyright (C) 2006 Gwenaelle MARCOU & Christophe LITZINGER
 *
 * This file is part of DICOM.
 *
 * DICOM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * DICOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DICOM; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */

/*======================================================================*/
/*                                                                      */
/*			Dictionnaire.cpp - Classe de gestion d'un dictionnaire		*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* ------------------------------------------------------------ INCLUDE */
/* ---------------------------------------------------- Include système */
#include <windows.h> // Application Windows
#include <shlobj.h> //Browser de dossier
#include <commctrl.h> // Common controls
#include <fstream> // Gestion des fichiers
#include <vector> // Gestion des vecteurs
#include <string> // Gestion des chaines de caracteres
#include <locale> // Gestion de isalnum
#include <time.h> // Gestion du temps
#include <algorithm> // Utilisation de sort

/* -------------------------------------------------- Include personnel */
#include "resource.h"
#include "Clavier.h"
#include "GestionnaireBoutons.h"
#include "Dictionnaire.h"
#include "Configuration.h"

/* -------------------------------------------------------------------- */
/*                        PRIVE                                         */
/* ------------------------------------------------ Constantes diverses */
#define MAX_WORD_LENGTH 64 // Taille maximale d'un mot dans le dictionnaire

/* -------------------------------------------------- Constantes dialog */
#define MAIN_DLG_DEFAULT_WIDTH 160 // Taille minimum en largeur du dialogue princ
#define MAIN_DLG_DEFAULT_HEIGHT 320 // Taille minimum en hauteur du dialogue princ
#define TITLE_BAR_HEIGHT 30 // Taille de la barre de titre d'une application
#define IDT_TIMER_HIDE 1 // ID du timer d'affichage

/* ----------------------------------------------------------- DLL Hook */
#define HOOKDLL_FILE_NAME "HookDLL.dll"
typedef void (*DLLInitHook)(HWND hDest); // Fonction d'init de HOOK
DLLInitHook InitHook = NULL; 
typedef void (*DLLEndHook)(); // Fonction de fin de HOOK
DLLEndHook EndHook = NULL;
HINSTANCE hinstDLLHook; // Instance la DLL de HOOK

/* ----------------------------------------------- Variables Graphiques */
HINSTANCE hInst; // Instance de notre application
HWND mainDlg = NULL; // Handle de notre dialogue principal
HWND configDlg = NULL; // Handle de notre dialogue de config
HWND dicoDlg = NULL; // Handle de notre dialogue de dico
HWND insertDlg = NULL; // Handle de notre dialogue d'insertion
HMENU mainDlgSubMenu = NULL; //Handle sur le menu minimise
int heightButtonDefault; // Hauteur d'un bouton au chargement
GestionnaireBoutons gestionnaireBoutons; // Gestionnaire des boutons de l'IHM

/* ------------------------------------------------ Variables recherche */
#define DICTIONARY_FILE_NAME "DICOM.dic"
Dictionnaire dictionnaire; // Dictionnaire des mots
std::string currentSearch; // Texte de recherche courant

/* ------------------------------------------- Gestion de configuration */
#define CONFIG_FILE_NAME "DICOM.cfg"
Configuration config(CONFIG_FILE_NAME);

/* ---------------------------------------------------------- Fonctions */

/*
 * @brief Insere une icone d'application dans la barre des taches.
 */
void addTaskBarIcon()
{
	// Structure d'insertion dans la barre des taches
	NOTIFYICONDATA ndata;
	ndata.cbSize = sizeof(NOTIFYICONDATA);
	ndata.hWnd = mainDlg;
	ndata.uID = ID_TASK_BAR;//Id unique de l'instance dans la task barre
	ndata.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;//Indique les champs valide
	ndata.uCallbackMessage = WM_COMMAND; // message qui sera envoyé à la fenêtre.
	ndata.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_APPLICATION)); // icone qui sera affiché dans la barre des taches

	// Definition de l'infobulle
	strcpy(ndata.szTip,"DICOM");

	// Ajout
	Shell_NotifyIcon(NIM_ADD, &ndata);
}

/*
 * @brief Supprime une icone d'application de la barre des taches.
 */
void removeTaskBarIcon()
{
	// Structure de suppression de la barre des taches
	NOTIFYICONDATA ndata;
	ndata.cbSize = sizeof(NOTIFYICONDATA); 
	ndata.hWnd = mainDlg;
	ndata.uID = ID_TASK_BAR; 

	// Suppression
	Shell_NotifyIcon(NIM_DELETE, &ndata);
}

/*
 * @brief Cette fonction a pour role de definir la taille du dialogue
 * principale en fonction du nombre de boutons a afficher et la taille
 * courante des boutons.
 * @param afficherDialogue Indique si l'affichage du dialogue doit avoir
 * lieu apres adaptation. TRUE le dialogue sera affiche.
 */
void adapterDialoguePrincipale(bool afficherDialogue = true)
{
	// Indique si cette adaptation est la premire (coin haut droite)
	static bool premiereAdaptation = true;
	if (true == premiereAdaptation) // Tous les boutons doivent etre initialise
		gestionnaireBoutons.SetNbBoutons(NB_MAX_BUTTONS);

	// Taille du premier bouton de l'interface
	RECT ctrlRect; GetClientRect(GetDlgItem(mainDlg, IDC_BUTTON_F1), &ctrlRect);

	// Affichage des boutons dont l'index est plus petit que GetNbBoutons()
	for (unsigned int i=0; i<NB_MAX_BUTTONS; ++i) {
		HWND hButton = GetDlgItem(mainDlg, i + IDC_BUTTON_F1);
		if (i < gestionnaireBoutons.GetNbBoutons()) { // Affichage du bouton
			SendMessage(hButton,WM_SETTEXT, 0, (WPARAM)NULL);
			ShowWindow(hButton, SW_SHOW); 
		}
		else // Hide du bouton
			ShowWindow(hButton, SW_HIDE); 
	}

	// Dimensionnement de la fenetre pour qu'elle accepte tous les boutons
	if (false == premiereAdaptation) {
		// Lecture des coordonnees du dialogue
		RECT dlgRect;
		GetWindowRect(mainDlg, &dlgRect);

		// Lecture des dimension de la zone client
		RECT dlgClientRect;
		GetClientRect(mainDlg, &dlgClientRect);

		// Redimensionnement du dialogue
		MoveWindow(mainDlg
			, dlgRect.left
			, dlgRect.top
			, dlgRect.right-dlgRect.left
			, ((dlgRect.bottom-dlgRect.top)-dlgClientRect.bottom)+20+(gestionnaireBoutons.GetNbBoutons()==0?1:gestionnaireBoutons.GetNbBoutons())*ctrlRect.bottom
			, TRUE);
	}
	else { // Application de la configuration
		if (0 == config.GetUnsignedIntParam(DIALOG_POS_X))
			config.SetUnsignedIntParam(DIALOG_POS_X, GetSystemMetrics(SM_CXSCREEN) - MAIN_DLG_DEFAULT_WIDTH);
		if (0 == config.GetUnsignedIntParam(DIALOG_POS_Y))
			config.SetUnsignedIntParam(DIALOG_POS_Y, TITLE_BAR_HEIGHT);
		if (0 == config.GetUnsignedIntParam(DIALOG_WIDTH))
			config.SetUnsignedIntParam(DIALOG_WIDTH, MAIN_DLG_DEFAULT_WIDTH);
		if (0 == config.GetUnsignedIntParam(DIALOG_HEIGHT))
			config.SetUnsignedIntParam(DIALOG_HEIGHT, MAIN_DLG_DEFAULT_HEIGHT);

		MoveWindow(mainDlg
			, config.GetUnsignedIntParam(DIALOG_POS_X)
			, config.GetUnsignedIntParam(DIALOG_POS_Y)
			, config.GetUnsignedIntParam(DIALOG_WIDTH)
			, (unsigned int)(config.GetUnsignedIntParam(DIALOG_HEIGHT)*((float)(gestionnaireBoutons.GetNbBoutons()==0?1:gestionnaireBoutons.GetNbBoutons())/NB_MAX_BUTTONS))
			, TRUE);

		premiereAdaptation=false;

		gestionnaireBoutons.SetNbBoutons(0); // Aucun boutons
	}		


	// Affichage de la fenetre si nbButtons > 0
	if (gestionnaireBoutons.GetNbBoutons() > 0) {
		// Affichage de la fenetre principale sans perdre la fenetre en foreground
		HWND currentHWND = GetForegroundWindow();
		if (afficherDialogue) ShowWindow(mainDlg, SW_SHOW);
		SetForegroundWindow(currentHWND);

		// Mise en place du timer de reduction
		SetTimer(mainDlg, IDT_TIMER_HIDE, config.GetUnsignedIntParam(SPLASH_TIME), (TIMERPROC)NULL);
	}
	else // Sinon on cache la fenetre principale
		ShowWindow(mainDlg, SW_HIDE);
}

/*
 * @brief Cette fonction a pour role de faire la recherche dans le dictionnaire de mot
 * de la variable globale "currentSearch". Le resultat de la recherche est affecte
 * au differents boutons en fonction de leur pertinence.
 */
void RechercherDictionnaire()
{
	// Effectue une recherche des que l'on depasse le nombre de lettres minimum
	if (config.GetUnsignedIntParam(SPLASH_NB_LETTERS) <= currentSearch.length()) {
		// Recherche des mots disponibles et sauvegarde du nombre de resultats
		std::vector<std::string> findWords;
		gestionnaireBoutons.SetNbBoutons(dictionnaire.FindWordsAtAllCost(findWords
			, NB_MAX_BUTTONS
			, currentSearch));

		// Affectation des resultats pour chaque boutons de l'interface
		for (unsigned int i=0; i<gestionnaireBoutons.GetNbBoutons(); ++i)
			gestionnaireBoutons.SetBoutonText(i, findWords[i]);
	}
	else
		gestionnaireBoutons.SetNbBoutons(0);

	// Raffraichissement final
	adapterDialoguePrincipale();
}

/*
 * @brief Cette fontion est associee au traitement des messages du dialogue
 * de gestion du dictionnaire.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
LRESULT CALLBACK DlgDicoProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_COMMAND: //Message de type command
		switch (LOWORD(wParam)) {

		case IDC_BUTTON_AJOUTER: { // Ajout d'un mot au dictionnaire
			// Lecture du texte dans l'edit
			char buffer[MAX_WORD_LENGTH];
			GetDlgItemText(hDlg, IDC_EDIT_MOT, buffer, sizeof(buffer));
			// On verifie que le mot existe pas deja
			std::string message;
			if (strlen(buffer)>=3) {
				// Mot inexistant
				if (!dictionnaire.ExisteMot(static_cast<std::string>(buffer))) {
					dictionnaire.AjouterMot(static_cast<std::string>(buffer));
					message = "\"";
					message.append(buffer);
					message.append("\" ajouté.");
					// Recherche dans le dico
					std::vector<std::string> findWords;
					dictionnaire.FindWords(findWords
						, 50
						, static_cast<std::string>(buffer)
						, false);
					// Suppression des mots deja present
					SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_RESETCONTENT, 0, 0);
					// Ajout des mots dans la liste
					for (std::vector<std::string>::const_iterator iter = findWords.begin();
						iter!=findWords.end();
						++iter) {
						SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)iter->c_str());
					}
				}
				else { // Deja existant
					message = "\"";
					message.append(buffer);
					message.append("\" existe déjà.");
				}
			}
			else {
				message.append("Veuillez saisir plus de 3 lettres.");
			}
			// Mise a jour du message d'information
			SetDlgItemText(hDlg, IDC_STATIC_RETOUR, message.c_str());
			}
			return TRUE;

		case IDC_BUTTON_IMPORTER: { // Importation d'un fichier dictionnaire
			OPENFILENAME fichier;
			char nom[MAX_PATH];
			fichier.lStructSize = sizeof(OPENFILENAME);
			fichier.hInstance = 0;
			fichier.hwndOwner = hDlg;
			fichier.lpstrFilter = "Dictionnaire\0*.dic\0\0";
			fichier.lpstrFile = nom;
			fichier.lpstrCustomFilter = 0;
			fichier.lpstrFileTitle = 0;
			fichier.nFileExtension = 0;
			fichier.nFileOffset = 0;
			fichier.lCustData = 0;
			fichier.lpTemplateName = 0;
			fichier.lpstrInitialDir = 0;
			fichier.lpstrDefExt = 0;
			fichier.lpfnHook = 0;
			fichier.nFileExtension = 0;
			fichier.nMaxCustFilter = 0;
			fichier.nMaxFileTitle = 0;
			fichier.nFilterIndex = 1; 
			fichier.nMaxFile = MAX_PATH;
			fichier.lpstrTitle = "Choisissez un fichier";
			fichier.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
			nom[0] = 0;
			if (GetOpenFileName(&fichier) == TRUE) {
				if (dictionnaire.LoadFromFile(nom, true) == TRUE)
					SetDlgItemText(hDlg, IDC_STATIC_RETOUR, "Dictionnaire importé.");
				else
					SetDlgItemText(hDlg, IDC_STATIC_RETOUR, "Erreur lors de l'importation.");
			}
			}
			return TRUE;

		case IDC_BUTTON_SUPPRIMER: { // Suppression d'un mot du dictionnaire
			int index = (int)SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_GETCURSEL, 0, 0);
			if (LB_ERR != index) {
				// Recuperation du mot
				char buffer[MAX_WORD_LENGTH];
				SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_GETTEXT, index, (LPARAM)(LPTSTR)buffer);
				// Suppression du mot de la liste
				SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_DELETESTRING, index, 0);
				// Suppression du mot dans le dico
				dictionnaire.SupprimerMot(static_cast<std::string>(buffer));
				SetDlgItemText(hDlg, IDC_STATIC_RETOUR, "Mot supprimé.");
			}
			}
			return TRUE;

		case IDC_BUTTON_RAZ_MOT: { // Remise a zero du poids d'un mot
			int index = (int)SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_GETCURSEL, 0, 0);
			if (LB_ERR != index) {
				// Recuperation du mot
				char buffer[MAX_WORD_LENGTH];
				SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_GETTEXT, index, (LPARAM)(LPTSTR)buffer);
				// RAZ poids du mot
				dictionnaire.ResetPoids(static_cast<std::string>(buffer));
				SetDlgItemText(hDlg, IDC_STATIC_RETOUR, "Poids du mot remis à zéro.");
			}
			}
			return TRUE;

		case IDC_BUTTON_RAZ_DICO: { // Suppression de tous les mots du dictionnaire
				if(MessageBox(mainDlg
					, "Etes-vous sûr de vouloir supprimer tous les mots ?"
					, "Confirmation"
					, MB_OKCANCEL | MB_TASKMODAL | MB_ICONQUESTION) == IDOK ) {
					HWND cacheDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_DEMARRAGE), mainDlg, (DLGPROC)NULL);
					dictionnaire.SupprimerMots();
					DestroyWindow(cacheDialog);
					SetDlgItemText(hDlg, IDC_STATIC_RETOUR, "Tous les mots du dictionnaire ont été supprimés.");
				}
			}
			return TRUE;

		case IDC_BUTTON_RAZ_POIDS: { // Remise a zero des poids du dictionnaire
				if(MessageBox(mainDlg
					, "Etes-vous sûr de vouloir remettre à zéro le poids de chaque mot ?"
					, "Confirmation"
					, MB_OKCANCEL | MB_TASKMODAL | MB_ICONQUESTION) == IDOK ) {
					HWND cacheDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_DEMARRAGE), mainDlg, (DLGPROC)NULL);
					dictionnaire.ResetPoids();
					DestroyWindow(cacheDialog);
					SetDlgItemText(hDlg, IDC_STATIC_RETOUR, "Tous les mots du dictionnaire sont remis à zéro.");
				}
			}
			return TRUE;

		case IDC_EDIT_MOT: { // Notification de modification, maj de la liste
			// Lecture du texte dans l'edit
			char buffer[MAX_WORD_LENGTH];
			GetDlgItemText(hDlg, IDC_EDIT_MOT, buffer, sizeof(buffer));
			// Recherche dans le dico
			std::vector<std::string> findWords;
			dictionnaire.FindWords(findWords
				, 50
				, static_cast<std::string>(buffer)
				, false);
			// Suppression des mots deja present
			SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_RESETCONTENT, 0, 0);
			// Ajout des mots dans la liste
			for (std::vector<std::string>::const_iterator iter=findWords.begin();
				iter!=findWords.end();
				++iter) {
				SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)iter->c_str());
			}
			}
			return TRUE;

		case IDOK: //Demande fermeture de la configuration
		case WM_DESTROY: 
		case WM_CLOSE:
			dicoDlg = NULL;
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/*
 * @brief Cette fonction est utilisee pour lister les polices d'affichages utilisable
 * sur la machine locale.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
int CALLBACK callbackfunc(ENUMLOGFONT FAR *p, NEWTEXTMETRIC FAR *tm, int type, LPARAM par) 
{
	std::deque <std::string> * dequeFonts = (std::deque <std::string> *)(par);
	dequeFonts->push_back(p->elfLogFont.lfFaceName);
	return 1 ;
} 

/*
 * @brief Cette fontion est associee au traitement des messages du dialogue de
 * configuration de l'application.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
LRESULT CALLBACK DlgConfigProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Variable utilisee pour simuler l'affichage d'un bouton
	static GestionnaireBoutons gestionnaireConfig;

	switch(uMsg) {

	case WM_INITDIALOG: { // Initialisation des champs du dialogue
		// Initialisation du gestionnaire d'apercu
		gestionnaireConfig.SetDialogue(hDlg);
		gestionnaireConfig.SetIdBase(IDC_BUTTON_APERCU);
		gestionnaireConfig.SetCouleurTexte(gestionnaireBoutons.GetCouleurTexte());
		gestionnaireConfig.SetCouleurFond(gestionnaireBoutons.GetCouleurFond());
		gestionnaireConfig.SetNbBoutons(1);
		gestionnaireConfig.SetPolice(gestionnaireBoutons.GetPolice());
		gestionnaireConfig.SetPoliceBold(gestionnaireBoutons.GetPoliceBold());
		gestionnaireConfig.SetPoliceItalic(gestionnaireBoutons.GetPoliceItalic());
		gestionnaireConfig.SetBoutonText(0, static_cast<std::string>("Aperçu"));
		gestionnaireConfig.AppliquerFont(false);
		// valeur pour les slides barres max
		SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)255);
		SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)255);
		SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)255);
		// valeur pour les slides barres min
		SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)0);
		SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)0);
		SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)0);
		// Selection bouton texte
		CheckDlgButton(hDlg, IDC_CHECK_GRAS, gestionnaireConfig.GetPoliceBold()?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_ITALIQUE, gestionnaireConfig.GetPoliceItalic()?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_RADIO_TEXTE, BST_CHECKED);
		SendMessage(hDlg, WM_COMMAND, IDC_RADIO_TEXTE, 0);
		// Enumeration des polices d'ecriture
		HDC hDC = GetDC(hDlg);
		std::deque <std::string> dequeFonts;
		EnumFontFamilies(hDC, NULL, (FONTENUMPROC)callbackfunc, (LPARAM)&dequeFonts);
		std::sort(dequeFonts.begin(),dequeFonts.end());
		ReleaseDC(hDlg, hDC);
		// Affichage dans le combox box
		for (std::deque<std::string>::iterator iter = dequeFonts.begin();
			iter!=dequeFonts.end();
			++iter) {
			SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)iter->c_str());
			//MessageBox(NULL, (*iter).c_str(), "Font", MB_OK);
		}
		// Selection de la police courante
		int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_FINDSTRINGEXACT, 0, (LPARAM)(LPTSTR)gestionnaireConfig.GetPolice().c_str());
		if (CB_ERR != index) {
			SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_SETCURSEL, index, 0);
		}
		char bufferConv[11];
		// Affichage du nombre de lettres avant affichage
		SetDlgItemText(hDlg, IDC_EDIT_NB_LETTRE, _ultoa(config.GetUnsignedIntParam(SPLASH_NB_LETTERS), bufferConv, 10));
		// Affichage du nombre de secondes avant cachage...
		SetDlgItemText(hDlg, IDC_EDIT_TEMPS, _ultoa(config.GetUnsignedIntParam(SPLASH_TIME) / 1000, bufferConv, 10));
		// Affichage du checkbox d'auto insertion
		CheckDlgButton(hDlg, IDC_CHECK_AUTOMATIQUE, config.GetBoolParam(SPLASH_AUTO_INSERT)?BST_CHECKED:BST_UNCHECKED);
		}
		return TRUE;

	case WM_DRAWITEM: { //Dessin des boutons
			// Structure d'information pour le dessin
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			if (lpdis->CtlID == IDC_BUTTON_APERCU) {
				// Bouton de l'interface
				gestionnaireConfig.DessinerBouton(lpdis, lpdis->CtlID-IDC_BUTTON_APERCU);
				return TRUE;
			}
		}
		break;

	case WM_HSCROLL: { // Mouvement horizontale d'une scollbar
		// Creation de la couleur RGB issue du mouvement
		COLORREF couleur;
		couleur = RGB(SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_GETPOS, 0, 0)
			, SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_GETPOS, 0, 0)
			, SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_GETPOS, 0, 0));
		// Valeur dans les textes box
		char bufferConv[11];
		SendDlgItemMessage(hDlg, IDC_EDIT_ROUGE, WM_SETTEXT, NULL, (LPARAM)itoa(GetRValue(couleur), bufferConv, 10));
		SendDlgItemMessage(hDlg, IDC_EDIT_VERT, WM_SETTEXT, NULL, (LPARAM)itoa(GetGValue(couleur), bufferConv, 10));
		SendDlgItemMessage(hDlg, IDC_EDIT_BLEU, WM_SETTEXT, NULL, (LPARAM)itoa(GetBValue(couleur), bufferConv, 10));
		// En fonction du checkbox on modifit les parametres associes
		if (IsDlgButtonChecked(hDlg, IDC_RADIO_TEXTE) == BST_CHECKED)
			gestionnaireConfig.SetCouleurTexte(couleur);
		else
			gestionnaireConfig.SetCouleurFond(couleur);
		// MAJ du bouton
		SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, true, 0);
		SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, false, 0);
		}
		break;

	case WM_COMMAND: //Message de type command
		switch (LOWORD(wParam)) {
		case IDC_COMBO_POLICE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {  //Changement d'onglet pour la police
				// Lecture de la police courante
				int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_GETCURSEL, 0, 0);
				if (CB_ERR != index) {
					char buffer[MAX_WORD_LENGTH];
					SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_GETLBTEXT, index, (LPARAM)(LPTSTR)buffer);
					gestionnaireConfig.SetPolice(static_cast<std::string>(buffer));
					gestionnaireConfig.AppliquerFont();
					SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, true, 0);
					SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, false, 0);
				}
				return TRUE;
			}
			break;

		case IDC_RADIO_TEXTE:
		case IDC_RADIO_FOND: {
			COLORREF couleur;
			// En fonction du checkbox on lit les parametres associes
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_TEXTE) == BST_CHECKED) {
				couleur = gestionnaireConfig.GetCouleurTexte();
			}
			else {
				couleur = gestionnaireConfig.GetCouleurFond();
			}
			// valeur pour la couleur
			SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)GetRValue(couleur));
			SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)GetGValue(couleur));
			SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)GetBValue(couleur));
			// Valeur dans les textes box
			char bufferConv[11];
			SendDlgItemMessage(hDlg, IDC_EDIT_ROUGE, WM_SETTEXT, NULL, (LPARAM)itoa(GetRValue(couleur), bufferConv, 10));
			SendDlgItemMessage(hDlg, IDC_EDIT_VERT, WM_SETTEXT, NULL, (LPARAM)itoa(GetGValue(couleur), bufferConv, 10));
			SendDlgItemMessage(hDlg, IDC_EDIT_BLEU, WM_SETTEXT, NULL, (LPARAM)itoa(GetBValue(couleur), bufferConv, 10));
			}
			break;

		case IDC_CHECK_GRAS: // Selection du type de police
		case IDC_CHECK_ITALIQUE: {
			gestionnaireConfig.SetPoliceBold(IsDlgButtonChecked(hDlg, IDC_CHECK_GRAS) == BST_CHECKED);
			gestionnaireConfig.SetPoliceItalic(IsDlgButtonChecked(hDlg, IDC_CHECK_ITALIQUE) == BST_CHECKED);
			gestionnaireConfig.AppliquerFont(); // Maj de la font
			SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, true, 0);
			SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, false, 0);
			}
			break;

		case IDOK: { //Demande fermeture de la configuration
			// Sauvegarde de la configuration de l'apercu pour l'application
			gestionnaireBoutons.SetCouleurTexte(gestionnaireConfig.GetCouleurTexte());
			gestionnaireBoutons.SetCouleurFond(gestionnaireConfig.GetCouleurFond());
			gestionnaireBoutons.SetPolice(gestionnaireConfig.GetPolice());
			gestionnaireBoutons.SetPoliceBold(gestionnaireConfig.GetPoliceBold());
			gestionnaireBoutons.SetPoliceItalic(gestionnaireConfig.GetPoliceItalic());
			gestionnaireBoutons.AppliquerFont();
			// Lecture du nombre de lettres avant affichage
			char bufferRead[11];
			GetDlgItemText(hDlg, IDC_EDIT_NB_LETTRE, bufferRead, sizeof(bufferRead));
			config.SetUnsignedIntParam(SPLASH_NB_LETTERS, atol(bufferRead));
			// Lecture du nombre de seconde avant cachage...
			GetDlgItemText(hDlg, IDC_EDIT_TEMPS, bufferRead, sizeof(bufferRead));
			config.SetUnsignedIntParam(SPLASH_TIME, atol(bufferRead) * 1000);
			// Affichage du checkbox d'auto insertion
			if (IsDlgButtonChecked(hDlg, IDC_CHECK_AUTOMATIQUE) == BST_CHECKED) config.SetBoolParam(SPLASH_AUTO_INSERT, true);
			else config.SetBoolParam(SPLASH_AUTO_INSERT, false);
			}
		case WM_DESTROY:
		case WM_CLOSE: {
			configDlg = NULL;
			EndDialog(hDlg, 0);
			}
			return TRUE;
		}
		break;		
	}

	return FALSE;
}

/*
 * @brief Cette fontion est associee au traitement des messages du dialogue
 * d'insertion automatique.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
LRESULT CALLBACK DlgInsertProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {

	case WM_INITDIALOG: // Initialisation des champs du dialogue
		// Timer avant fermeture
		SetTimer(hDlg, IDT_TIMER_HIDE, config.GetUnsignedIntParam(SPLASH_TIME), (TIMERPROC)NULL);
		SetDlgItemText(hDlg, IDC_STATIC_INSERER, currentSearch.c_str());
		return TRUE;

	case WM_TIMER: // Reception d'un evenement timer 
		switch (wParam) { 
		case IDT_TIMER_HIDE: // Disparision de la fenetre 
			KillTimer(hDlg, IDT_TIMER_HIDE);
			insertDlg = NULL;
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;

	case WM_COMMAND: //Message de type command
		switch (LOWORD(wParam)) {
		case IDOK: { //Demande fermeture de la configuration
			char bufferRead[MAX_WORD_LENGTH];
			GetDlgItemText(hDlg, IDC_STATIC_INSERER, bufferRead, sizeof(bufferRead) -1);
			// Ajout du mot au dictionnaire
			if (!dictionnaire.ExisteMot(static_cast<std::string>(bufferRead)))
				dictionnaire.AjouterMot(static_cast<std::string>(bufferRead));
			}
		case WM_DESTROY:
		case WM_CLOSE:
			KillTimer(hDlg, IDT_TIMER_HIDE);
			insertDlg = NULL;
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;		
	}

	return FALSE;
}

/*
 * @brief Cette fontion est associee au traitement des messages du dialogue principal.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
LRESULT CALLBACK DlgMainProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
	case WM_INITDIALOG: //Initialisation du dialogue
		return TRUE;

	case WM_TIMER: // Reception d'un evenement timer 
		switch (wParam) { 
		case IDT_TIMER_HIDE: {// Disparision de la fenetre principale
				// On cache la fenetre principale
				ShowWindow(mainDlg, SW_HIDE);

				// Destruction du timer de hide
				KillTimer(hDlg, IDT_TIMER_HIDE);
			}
			break;
		}
		break;

	case WM_DRAWITEM: { //Dessin des boutons
			// Structure d'information pour le dessin
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
			if ((lpdis->CtlID>=IDC_BUTTON_F1)&&(lpdis->CtlID<(IDC_BUTTON_F1+NB_MAX_BUTTONS))) {
				// Bouton de l'interface
				gestionnaireBoutons.DessinerBouton(lpdis, lpdis->CtlID-IDC_BUTTON_F1);
				return TRUE;
			}
		}
		break;

	case WM_COMMAND: //Message de type command
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_F1:
		case IDC_BUTTON_F2:
		case IDC_BUTTON_F3:
		case IDC_BUTTON_F4:
		case IDC_BUTTON_F5:
		case IDC_BUTTON_F6:
		case IDC_BUTTON_F7:
		case IDC_BUTTON_F8:
		case IDC_BUTTON_F9:
		case IDC_BUTTON_F10:
			// N'autorise que les bouttons affiches
			if (LOWORD(wParam)-IDC_BUTTON_F1 < (int)gestionnaireBoutons.GetNbBoutons()) {
				dictionnaire.IncrementerPoids(gestionnaireBoutons.GetBoutonText(LOWORD(wParam)-IDC_BUTTON_F1
						, ""));
				// Simulation du texte au clavier
				Clavier::SimulerTexte(
					gestionnaireBoutons.GetBoutonText(LOWORD(wParam)-IDC_BUTTON_F1
						, currentSearch).c_str());
			}
			return TRUE;

		case WM_DESTROY: //Demande fermeture de l'application
		case WM_CLOSE:
			// On cache la fenetre principale
			ShowWindow(mainDlg, SW_HIDE);
			return TRUE;

		default:
		switch((UINT)lParam) {

			case WM_RBUTTONDOWN: { //Affichage d'un popup menu
				//Recuperation de la position de la souris
				POINT pt;
				GetCursorPos(&pt);

				//Affichage du menu et attente d'un choix
				int i = (int)TrackPopupMenu(mainDlgSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hDlg, 0);
				switch(i) {
					case ID_POPUP_CONFIGURER: // Affichage de la fenetre de configuration
						if (NULL == configDlg) {
							configDlg = CreateDialog(hInst
								, MAKEINTRESOURCE(IDD_DIALOG_CONFIGURER)
								, mainDlg
								, (DLGPROC)DlgConfigProc);
						}
						return TRUE;
					case ID_POPUP_DICTIONNAIRE: // Affichage de la fenetre de dictionnaire
						if (NULL == dicoDlg) {
							dicoDlg = CreateDialog(hInst
								, MAKEINTRESOURCE(IDD_DIALOG_DICTIONNAIRE)
								, mainDlg
								, (DLGPROC)DlgDicoProc);
						}
						return TRUE;
					case ID_POPUP_QUITTER: // On quitte l'application
						// Recuperation des informations d'affichage de la fenetre
						ShowWindow(hDlg,SW_HIDE);
						gestionnaireBoutons.SetNbBoutons(NB_MAX_BUTTONS);
						adapterDialoguePrincipale(false);
						// Recuperation de la taille du dialogue courant
						RECT dlgRect;
						GetWindowRect(hDlg,&dlgRect);
						// Sauvegarde des informations dans la configuration
						config.SetStringParam(FONT_TYPE, gestionnaireBoutons.GetPolice());
						config.SetUnsignedIntParam(FONT_TEXT_COLOR, gestionnaireBoutons.GetCouleurTexte());
						config.SetUnsignedIntParam(DIALOG_BACKGROUND_COLOR, gestionnaireBoutons.GetCouleurFond());
						config.SetBoolParam(FONT_BOLD, gestionnaireBoutons.GetPoliceBold());
						config.SetBoolParam(FONT_ITALIC, gestionnaireBoutons.GetPoliceItalic());
						// Sauvegarde des dimensions du dialogue principal
						config.SetUnsignedIntParam(DIALOG_POS_X, dlgRect.left);
						config.SetUnsignedIntParam(DIALOG_POS_Y, dlgRect.top);
						config.SetUnsignedIntParam(DIALOG_WIDTH, dlgRect.right - dlgRect.left);
						config.SetUnsignedIntParam(DIALOG_HEIGHT, dlgRect.bottom - dlgRect.top);
						// Suppression de l'icone de la barre des taches
						removeTaskBarIcon();
						KillTimer(hDlg, IDT_TIMER_HIDE);
						EndDialog(hDlg, 0);
						PostQuitMessage(0); // Fin de l'application
						return TRUE;
					case ID_POPUP_ANNULER:
						return TRUE;
				}
			}
			break;
			}
			break;
		}
		break;

	case PM_AUTOINSERT: // Gere la validation d'une insertion automatique
		switch (wParam) {
		case INSERTWORD: // Ajoute le mot en attente d'insertion (cf. dialog)
			if (NULL != insertDlg)
				SendMessage(insertDlg, WM_COMMAND, IDOK, 0L);
			return TRUE;
		}
		return TRUE;

	case PM_AFFICHERRECHERCHE: //Demande issu d'une saisie clavier (DLLHOOK)
		switch (wParam) {
		case RESETWORD : // Met a zero le buffer de recherche
			// Affichage d'un dialogue pour l'insertion du mot dans le dico
			if (config.GetBoolParam(SPLASH_AUTO_INSERT) && (currentSearch.length() >= 3)) {
				
				// Traitement des caracteres pourris en fin de recherche
				switch (currentSearch[currentSearch.length()-1]) {
					case '.' : // Retrait du dernier caractere
						currentSearch.resize(currentSearch.length()-1);
						break;
				}

				// Si le mot n'existe pas dans le dictionnaire
				if ((NULL == insertDlg) && (dictionnaire.ExisteMot(currentSearch) == false)) {
					HWND before = GetForegroundWindow();
					insertDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_INSERTION), hDlg, (DLGPROC)DlgInsertProc);
					SetForegroundWindow(insertDlg);
					
					// Lecture des coordonnees du dialogue
					RECT dlgRect;
					GetWindowRect(insertDlg, &dlgRect);

					// Positionnement du dialogue en bas a droite
					MoveWindow(insertDlg
						, GetSystemMetrics(SM_CXSCREEN) - (dlgRect.right-dlgRect.left)
						, GetSystemMetrics(SM_CYSCREEN) - (dlgRect.bottom-dlgRect.top + 35)
						, dlgRect.right-dlgRect.left
						, dlgRect.bottom-dlgRect.top
						, TRUE);

					// Remise en place de la fenetre d'avant
					SetForegroundWindow(before);
				}
			}

			// Reset de la recherche courante
			currentSearch.clear();
			// On raffraichit la fenetre d'affichage
			gestionnaireBoutons.SetNbBoutons(0);
			adapterDialoguePrincipale();
			return TRUE;

		case APPENDCHAR : {// Ajoute un caractere au mot de recherche
			// Destruction du dialogue d'insertion
			if (NULL != insertDlg) {
				DestroyWindow(insertDlg);
				insertDlg = NULL;
			}
			// Recomposition de la lettre recue
			char buffer[2];
			memset(buffer, 0, sizeof(buffer));
			buffer[0] = (char)lParam;

			// Si le caractere est particulier
			switch(buffer[0]) {
				case '&':
				case '"':
				case '\'':
				case '(':
				case '_':
				case ')':
				case '=':
				case ',':
				case ';':
				case ':':
				case '!':
				case '?':
				case '/':
				case '§':
				case '%':
				case 'µ':
				case '£':
					SendMessage(hDlg, PM_AFFICHERRECHERCHE, (WPARAM)RESETWORD, 0L);
					return TRUE;
			}

			// Ajout de la lettre au texte de recherche courant
			currentSearch.append(buffer);

			// Lancement de la recherche
			RechercherDictionnaire();
			}
			return TRUE;

		case DELETELASTCHAR : // Supprime le dernier caractere de la recherche
			if (currentSearch.length() > 0) {
				// Retrait du dernier caractere
				currentSearch.resize(currentSearch.length()-1);

				// Lancement de la recherche
				RechercherDictionnaire();
			}
			return TRUE;
		}
		break;

	case WM_SIZING: { //Sur une action de redimensionnement
		// Lecture des coordonnees du dialogue
		RECT dlgRect;
		GetWindowRect(hDlg, &dlgRect);

		// Lecture des dimension de la zone client
		RECT dlgClientRect;
		GetClientRect(hDlg, &dlgClientRect);

		//Hauteur minimum atteinte
		int hMin = ((dlgRect.bottom-dlgRect.top)-dlgClientRect.bottom)+20+(gestionnaireBoutons.GetNbBoutons()==0?1:gestionnaireBoutons.GetNbBoutons())*heightButtonDefault;
		if ((((RECT*)lParam)->bottom - ((RECT*)lParam)->top) < hMin) {
			((RECT*)lParam)->bottom = ((RECT*)lParam)->top+(long)(hMin);
		}

		//Largueur minimal atteinte
		if ((((RECT*)lParam)->right - ((RECT*)lParam)->left) < MAIN_DLG_DEFAULT_WIDTH) {
			((RECT*)lParam)->right = ((RECT*)lParam)->left+MAIN_DLG_DEFAULT_WIDTH;
		}
		}
		return TRUE;

	case WM_SIZE: // Action aprés redimensionnement
		gestionnaireBoutons.PositionnerBoutons();
		return TRUE;
	}
	return FALSE;
}

/*
 * @brief Point d'entree de l'application windows DICOM.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{   
	/*********************VARIABLES GLOBALES*****************/
	hInst = hInstance;
	
	/*********************DLL HOOK***************************/
	{
		bool loaded = false;
		hinstDLLHook = LoadLibrary(HOOKDLL_FILE_NAME);
		if (NULL != hinstDLLHook) { // Chargement de la librairie HookDLL.dll ok
			// Recuperation de la fonction d'initialisation des hooks
			InitHook = (DLLInitHook)GetProcAddress(hinstDLLHook, "InitHook");
			if (NULL != InitHook) { // Importation de fonction ok
				// Recuperation de la fonction de suppression des hooks
				EndHook = (DLLEndHook)GetProcAddress(hinstDLLHook, "EndHook");
				if (NULL != EndHook) {// Importation de fontion ok
					// Tous les chargement ont reussit
					loaded = true;
				}
			}
		}

		if (false == loaded) {
			MessageBox(NULL
				, TEXT("Erreur lors du chargement de \"")
				  TEXT(HOOKDLL_FILE_NAME)
				  TEXT("\", veuillez verifier votre installation.")
				, "DLL introuvable"
				, MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return 1;
		}
	}

	/*********************CONFIGURATION***********************/
	if (config.GetStringParam(FONT_TYPE).compare("NULL") != 0) {
		gestionnaireBoutons.SetPolice(config.GetStringParam(FONT_TYPE));
		gestionnaireBoutons.SetPoliceBold(config.GetBoolParam(FONT_BOLD));
		gestionnaireBoutons.SetPoliceItalic(config.GetBoolParam(FONT_ITALIC));
		gestionnaireBoutons.SetCouleurTexte(config.GetUnsignedIntParam(FONT_TEXT_COLOR));
		gestionnaireBoutons.SetCouleurFond(config.GetUnsignedIntParam(DIALOG_BACKGROUND_COLOR));
	}

	/*********************DICTIONNAIRE***********************/
	{
		clock_t tempsAvant = clock();
		HWND cacheDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_DEMARRAGE), mainDlg, (DLGPROC)NULL);
		bool loaded = dictionnaire.LoadFromFile(DICTIONARY_FILE_NAME);
		clock_t tempsAttente = 1000 - (clock() - tempsAvant);
		if (tempsAttente > 0) Sleep(tempsAttente); // Affichage mini de 1s
		DestroyWindow(cacheDialog);

		if (false == loaded) { // Mode degrade
			MessageBox(NULL
				, TEXT("DICOM n'a pas reussit à trouver de fichier \".dic\". ")
				  TEXT("Aucun mot ne vous sera proposé.")
				, "Dictionnaire introuvable"
				, MB_ICONERROR | MB_OK | MB_TASKMODAL);
		}
	}

	/*********************FENETRE PRINCIPALE*****************/
	{
		mainDlg = CreateDialog(hInst
			, MAKEINTRESOURCE(IDD_MAIN_DIALOG)
			, NULL
			, (DLGPROC)DlgMainProc);

		// Sauvegarde de la hauteur d'un bouton au demarrage.
		// Cette valeur est utilisee pour interdire une trop grande reduction
		// de la fenetre principale.
		RECT ctrlRect; GetClientRect(GetDlgItem(mainDlg,IDC_BUTTON_F1),&ctrlRect);
		heightButtonDefault = ctrlRect.bottom;

		// Mise en place des parametres de gestion d'interface
		gestionnaireBoutons.SetDialogue(mainDlg);
		gestionnaireBoutons.SetIdBase(IDC_BUTTON_F1);

		// Dimensionnement du dialogue principal
		adapterDialoguePrincipale(false); // Initialisation de variables
		adapterDialoguePrincipale(); // Adaptation finale

		// Creation d'un icone d'application dans la barre des taches
		mainDlgSubMenu = GetSubMenu(LoadMenu(hInst, (LPCTSTR)IDR_MENU_MINIMIZE), 0);
		addTaskBarIcon();	
	}

	/*********************APPLICATION************************/
	InitHook(mainDlg);
	MSG msg;
	while(GetMessage(&msg,NULL,0,0) == TRUE) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/*********************LIBERATION DES RESSOURCES**********/
	// Liberation de la DLL de hook
	EndHook();
	FreeLibrary(hinstDLLHook);

	// Liberation des ressources graphiques
	DestroyMenu(mainDlgSubMenu);

	// Sauvegarde du contexte applicatif
	dictionnaire.SaveIntoFile(DICTIONARY_FILE_NAME);
	config.SaveIntoFile(CONFIG_FILE_NAME);

	return (int)msg.wParam;
}