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

/* -------------------------------------------------- Include personnel */
#include "Outils.h"
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define NAME_OK 0x99
#define SAUT L"\n"
//#pragma execution_character_set("utf-8") 

WNDPROC olvproc;
HWND lvh;
WNDPROC olvhproc;
bool name_ok = FALSE;
/* ---------------------------------------------------------- Fonctions */

LRESULT LVProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		// WM_NOTIFY
	case WM_NOTIFY:
	{
		// Catch the resize messages for the ListView Header and abort the resizing
		NMHDR *nmhdr = (NMHDR*)lParam;
		switch (nmhdr->code) {
		case HDN_BEGINTRACK:
			return TRUE;
		}
	}
	break;

	/*case WM_LBUTTONDOWN:*/

	}

	// Send all other messages to the ListViews original WndProc
	return CallWindowProc(olvproc, hWnd, uMsg, wParam, lParam);
}

LRESULT LVHProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		// Stop the header from changing to the "change column width mouse cursor"
		// Without this it will look like the user can resize, but he won't be able to do it
	case WM_SETCURSOR:
	{
		return TRUE;
	}

	// Stop the user from resizing by double clicking on the header
	case WM_LBUTTONDBLCLK:
	{
		return 0;
	}
	}

	// Send all other messages to the ListView Headers original WndProc
	return CallWindowProc(olvhproc, hWnd, uMsg, wParam, lParam);
}

void CreateLV(HWND hDlg) {
	DWORD dwStyle;
	hListWord = GetDlgItem(hDlg, IDC_LIST_DICO);
	wchar_t buf[256];
	LVCOLUMN lvc;
	memset(&lvc, 0, sizeof(LVCOLUMN));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	wsprintf(buf, L"%s", GetXMLString(STR_WORD).c_str());
	lvc.pszText = &buf[0];
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 179;
	SendMessage(hListWord, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

	wsprintf(buf, L"%s", GetXMLString(STR_POND).c_str());
	lvc.pszText = &buf[0];
	lvc.cx = 47;
	lvc.fmt = LVCFMT_RIGHT;
	SendMessage(hListWord, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);

	SetFocus(hListWord);
	SendMessage(hDlg, WM_SETREDRAW, TRUE, 0);

	olvproc = (WNDPROC)SetWindowLong(hListWord, GWLP_WNDPROC, (LONG_PTR)LVProc);

	lvh = ListView_GetHeader(hListWord);
	olvhproc = (WNDPROC)SetWindowLong(lvh, GWLP_WNDPROC, (LONG_PTR)LVHProc);

	dwStyle = SendMessage(hListWord, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
	dwStyle |= LVS_EX_FULLROWSELECT;
	SendMessage(hListWord, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);
}

void FillDicoCombo(HWND hDlg) {
	SendDlgItemMessage(dicoDlg, IDC_COMBO_CHOICE_DICO, CB_RESETCONTENT, 0, 0);

	std::deque<wstring> dicoFiles = GetDicoFiles();

	for (std::deque<wstring>::iterator iter = dicoFiles.begin();
		iter != dicoFiles.end();
		++iter) {
			SendDlgItemMessage(hDlg, IDC_COMBO_CHOICE_DICO, CB_ADDSTRING, 0, (LPARAM)iter->c_str());
		
	}
	if (the_dico_filename.empty())
		the_dico_filename = config.GetStringParam(DICTIONNARY_FILE_NAME).c_str();
	int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_CHOICE_DICO, CB_FINDSTRINGEXACT, 0, (LPARAM)the_dico_filename.c_str());
	if (CB_ERR != index) {
		SendDlgItemMessage(hDlg, IDC_COMBO_CHOICE_DICO, CB_SETCURSEL, index, 0);
	}
}

LRESULT CALLBACK NewDicoNameProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM) GetXMLString(STR_TITLE_NEW_DICO).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_NEW_DICO_NAME, GetXMLString(STR_NEW_DICO_NAME).c_str());
		break;

	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDOK:
			wchar_t buf[256];
			GetDlgItemText(hDlg, IDC_EDIT_NEW_DICO_NAME, buf, sizeof(buf));
			if (wcslen(buf) > 0) {
				if (!ExistFile(buf)) {
					the_dico_filename = buf;
					SendMessage(newDicoDlg, PM_NAME_OK, 0, 0);
					EndDialog(hDlg, 0);
					return TRUE;
				}
				else {
					int rep = MessageBox(hDlg, GetXMLString(STR_NEW_DICO_WARNING_ALREADY_EXIST).c_str(),
						GetXMLString(STR_TITLE_WARNING).c_str(),
						MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
					if (rep == IDYES) {
						the_dico_filename = buf;
						SendMessage(newDicoDlg, PM_NAME_OK, 0, 0);
						EndDialog(hDlg, 0);
						return TRUE;
					}
				}
			}
			else {
				MessageBox(hDlg, GetXMLString(STR_NEW_DICO_WARNING_NAME).c_str(),
					GetXMLString(STR_TITLE_WARNING).c_str(),
					MB_OK | MB_ICONWARNING);
			}
			break;

		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
	}
	}
	return FALSE;
}

LRESULT CALLBACK NewDicoProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG: {
		SetWindowText(hDlg, GetXMLString(STR_TITLE_NEW_DICO).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_NEW_DICO_CREATE, GetXMLString(STR_NEW_DICO_STATIC).c_str());
		SetDlgItemText(hDlg, IDC_RADIO_VIRGIN, GetXMLString(STR_NEW_DICO_VIRGIN).c_str());
		SetDlgItemText(hDlg, IDC_RADIO_FROM_ACTUAL, GetXMLString(STR_NEW_DICO_FROM_ACTUAL).c_str());
		SetDlgItemText(hDlg, IDC_RADIO_FROM_DEFAULT, GetXMLString(STR_NEW_DICO_FROM_DEFAULT).c_str());
		SetDlgItemText(hDlg, IDC_RADIO_FROM_FILE, GetXMLString(STR_NEW_DICO_FROM_FILE).c_str());
		SetDlgItemText(hDlg, IDCANCEL, GetXMLString(STR_CANCEL).c_str());
		CheckDlgButton(hDlg, IDC_RADIO_FROM_DEFAULT, BST_CHECKED);
		break; 
	}
	case PM_NAME_OK: {
		if (IsDlgButtonChecked(hDlg, IDC_RADIO_VIRGIN) == BST_CHECKED) {
			dictionnaire.SupprimerMots();
			SaveDico(the_dico_filename);
		}
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO_FROM_ACTUAL) == BST_CHECKED) {
			SaveDico(the_dico_filename);
		}
		else if (IsDlgButtonChecked(hDlg, IDC_RADIO_FROM_DEFAULT) == BST_CHECKED) {
			dictionnaire.LoadFromFile(DEFAULT_DICTIONARY_FILE);
			SaveDico(the_dico_filename);
		}
		config.SetStringParam(DICTIONNARY_FILE_NAME, the_dico_filename);
		SendMessage(dicoDlg, PM_CREATE_OK, 0, 0);
		EndDialog(hDlg, 0);
		newDicoDlg = NULL;
		return TRUE;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDOK:
			SaveDico(the_dico_filename);
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_FROM_FILE) == BST_CHECKED) {
				OPENFILENAME ofn = { 0 };
				wchar_t wFile[256];
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hDlg;
				ofn.lpstrFile = wFile;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(wFile);
				ofn.lpstrFilter = TEXT("*.dic\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				/*OPENFILENAME fichier = { 0 };
				wchar_t * nom;
				fichier.lStructSize = sizeof(OPENFILENAME);
				fichier.hInstance = 0;
				fichier.hwndOwner = hDlg;
				fichier.lpstrFilter = L"All Files\0*.*\0\0";
				fichier.lpstrFile = nom;
				fichier.lpstrCustomFilter = 0;
				fichier.lpstrFileTitle = 0;
				fichier.nFileExtension = 0;
				fichier.nFileOffset = 0;
				fichier.lCustData = 0;
				fichier.lpTemplateName = 0;
				fichier.lpstrInitialDir = NULL;
				fichier.lpstrDefExt = NULL;
				fichier.lpfnHook = 0;
				fichier.nFileExtension = 0;
				fichier.nMaxCustFilter = 0;
				fichier.nMaxFileTitle = 0;
				fichier.nFilterIndex = 1;
				fichier.nMaxFile = MAX_PATH;
				fichier.lpstrTitle = GetXMLString(STR_POPUP_CHOSE_FILE).c_str();
				fichier.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;*/
				if (GetOpenFileName(&ofn) == TRUE) {
					if (LoadDico(wFile, true) == TRUE) {
						wchar_t buf[2048];
						GetParsedPath(wFile, PATH_FILE, buf);
						the_dico_filename = buf;
						MoveToExeDirectory();
						SetDlgItemText(dicoDlg, IDC_STATIC_RETOUR, GetXMLString(STR_DICO_NOTIF_LOADED).c_str());
						SaveDico(the_dico_filename);
					}
					else
						SetDlgItemText(dicoDlg, IDC_STATIC_RETOUR, GetXMLString(STR_DICO_NOTIF_IMPORT_ERROR).c_str());

					SendMessage(dicoDlg, PM_CREATE_OK, 0, 0);
					EndDialog(hDlg, 0);
					newDicoDlg = NULL;
				}
			}

			else {
				HWND DlgDicoFileName = CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_DIALOG_DICO_NAME),
					hDlg,
					(DLGPROC)NewDicoNameProc);
				ShowWindow(DlgDicoFileName, WS_VISIBLE);
			}
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			newDicoDlg = NULL;
			return TRUE;

		}
		break;
	}
	}
	return FALSE;
}

/*
 * @brief Cette fontion est associee au traitement des messages du dialogue
 * de gestion du dictionnaire.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
LRESULT CALLBACK DlgDicoProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	std::locale loc("French_France");
	int iIndex;
	LPNMLISTVIEW pnm; 
	LPNMHDR hdr;
	
	switch(uMsg) {
	case WM_INITDIALOG: {

		EnableCharHook(false);
		SendMessage(mainDlg, PM_AFFICHERRECHERCHE, RESETWORD, 0);
		SetWindowText(hDlg, GetXMLString(STR_TITLE_DICO).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_MANAGE_DICO, GetXMLString(STR_DICO_MANAGE).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_MANAGE_WORD, GetXMLString(STR_DICO_MANAGE_WORD).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_CHOICE_DICO, GetXMLString(STR_DICO_CHOSE).c_str());
		SetDlgItemText(hDlg, IDC_BUTTON_LOAD_DICO, GetXMLString(STR_DICO_LOAD).c_str());
		SetDlgItemText(hDlg, IDC_BUTTON_NEW_DICO, GetXMLString(STR_DICO_NEW).c_str());
		SetDlgItemText(hDlg, IDC_BUTTON_DELETE_DICO, GetXMLString(STR_DICO_DELETE).c_str());
		SetDlgItemText(hDlg, IDC_BUTTON_RAZ_POIDS, GetXMLString(STR_DICO_RAZ_DICO).c_str());
		SetDlgItemText(hDlg, IDC_BUTTON_AJOUTER, GetXMLString(STR_DICO_ADD_WORD).c_str());
		SetDlgItemText(hDlg, IDC_BUTTON_MODIFY, GetXMLString(STR_DICO_MODIFY_WORD).c_str());
		SetDlgItemText(hDlg, IDC_BUTTON_SUPPRIMER, GetXMLString(STR_DICO_DELETE_WORD).c_str());
		SetDlgItemText(hDlg, IDC_CHECK_AUTO_INSERT, GetXMLString(STR_CONFIG_ACCESS_INSERT).c_str());
		SendDlgItemMessage(hDlg, IDC_EDIT_MOT, EM_SETCUEBANNER, TRUE, (LPARAM)GetXMLString(STR_DICO_CUE_WORD).c_str());
		SendDlgItemMessage(hDlg, IDC_EDIT_POIDS, EM_SETCUEBANNER, TRUE, (LPARAM)GetXMLString(STR_POND).c_str());
		  
		SendDlgItemMessage(hDlg, IDC_EDIT_POIDS, EM_SETREADONLY, TRUE, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_AJOUTER), false);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SUPPRIMER), false);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MODIFY), false);

		// Affichage du checkbox d'auto insertion
		CheckDlgButton(hDlg, IDC_CHECK_AUTO_INSERT, config.GetBoolParam(SPLASH_AUTO_INSERT) ? BST_CHECKED : BST_UNCHECKED);

		SendMessage(hDlg, PM_FILLDICOCOMBO, 0, 0);

		CreateLV(hDlg);
	}
	break;
	case WM_NOTIFY: 
		switch (LOWORD(wParam)) {
		case IDC_LIST_DICO:
			pnm = (LPNMLISTVIEW)lParam;
			if (pnm->hdr.hwndFrom == hListWord)
			{
				switch (pnm->hdr.code) {
				case NM_CUSTOMDRAW:
					SetWindowLong(hDlg, DWL_MSGRESULT, (LONG)TableDraw(lParam));
					return TRUE;
				case NM_KILLFOCUS:
					ListView_SetBkColor(GetDlgItem(hDlg, IDC_LIST_DICO), RGB(255, 255, 255));
					//SetWindowLong(hDlg, DWL_MSGRESULT, (LONG)TableDraw(lParam));
					return TRUE;
				case LVN_ITEMCHANGED:
					SendDlgItemMessage(hDlg, IDC_EDIT_POIDS, EM_SETREADONLY, FALSE, 0);
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SUPPRIMER), true);
					return TRUE;
				}
			}
			break;
		default: return FALSE;
		}
		return FALSE;

	case PM_CREATE_OK:
		SetDlgItemText(hDlg, IDC_STATIC_RETOUR, GetXMLString(STR_DICO_NOTIF_CREATED).c_str());
		SendMessage(dicoDlg, PM_FILLDICOCOMBO, 0, 0);
		return TRUE;
	
	case PM_FILLDICOCOMBO:
		FillDicoCombo(hDlg);
		return TRUE;

	case WM_COMMAND: //Message de type command
		if (debug) OutputDebugStringA("DICO_WM_COMMAND\n");
		switch (LOWORD(wParam)) {

		case IDC_BUTTON_LOAD_DICO:  {
			wchar_t buffer[MAX_PATH];
			iIndex = SendDlgItemMessage(hDlg, IDC_COMBO_CHOICE_DICO, CB_GETCURSEL, 0, 0);

			if (CB_ERR != iIndex) {
				SendDlgItemMessage(hDlg, IDC_COMBO_CHOICE_DICO, CB_GETLBTEXT, iIndex, (LPARAM)buffer);
				if (the_dico_filename.compare(buffer) != 0) {
					if(!the_dico_filename.empty()) SaveDico(the_dico_filename);
					the_dico_filename = buffer;
					LoadDico(the_dico_filename);
					SendMessage(hDlg, PM_FILLDICOCOMBO, 0, 0);
					SetDlgItemText(hDlg, IDC_STATIC_RETOUR, GetXMLString(STR_DICO_NOTIF_LOADED).c_str());
				}
			}
			return TRUE;
		}
		case IDC_BUTTON_NEW_DICO: {
			if (NULL == newDicoDlg) {
				newDicoDlg = CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_DIALOG_NEW_DICO),
					hDlg,
					(DLGPROC)NewDicoProc);
				ShowWindow(newDicoDlg, WS_VISIBLE);
			}
		}
		return TRUE;

		case IDC_BUTTON_DELETE_DICO: {
			iIndex = SendDlgItemMessage(hDlg, IDC_COMBO_CHOICE_DICO, CB_GETCURSEL, 0, 0);
			if (CB_ERR != iIndex) {
				wchar_t buffer[256];
				SendDlgItemMessage(hDlg, IDC_COMBO_CHOICE_DICO, CB_GETLBTEXT, iIndex, (LPARAM)buffer);
				if (the_dico_filename.compare(buffer) == 0) {
					MessageBox(hDlg,
						GetXMLString(STR_DICO_WARNING_NOTTHISONE).c_str(),
						GetXMLString(STR_TITLE_WARNING).c_str(),
						MB_OK | MB_ICONWARNING);
					return TRUE;
				}
				if (MessageBox(hDlg,
					GetXMLString(STR_DICO_WARNING_DELETE).c_str(),
					GetXMLString(STR_DICO_WARNING_DELETE_TITLE).c_str(),
					MB_YESNO | MB_ICONWARNING) == IDYES) {
					dictionnaire.DeleteDico(buffer);
					SendMessage(hDlg, PM_FILLDICOCOMBO, 0, 0);
					SetDlgItemText(hDlg, IDC_STATIC_RETOUR, GetXMLString(STR_DICO_NOTIF_DICO_DELETE).c_str());
				}
			}
		}
		return TRUE;
		case IDC_BUTTON_RAZ_POIDS: { // Remise a zero des poids du dictionnaire
			if (debug) OutputDebugStringA("	IDC_BOUTON_RAZ_POIDS\n");
			if (MessageBox(hDlg
				, GetXMLString(STR_DICO_WARNING_RAZ_POND).c_str()
				, GetXMLString(STR_DICO_WARNING_RAZ_POND_TITLE).c_str()
				, MB_OKCANCEL | MB_TASKMODAL | MB_ICONQUESTION) == IDOK) {
				HWND cacheDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_DEMARRAGE), mainDlg, (DLGPROC)NULL);
				dictionnaire.ResetPoids();
				DestroyWindow(cacheDialog);
				SetDlgItemText(hDlg, IDC_STATIC_RETOUR, GetXMLString(STR_DICO_NOTIF_RAZ_POND).c_str());
			}
		}
		return TRUE;
		case IDC_EDIT_MOT: { // Notification de modification, maj de la liste
			if (debug) OutputDebugStringA("	IDC_BOUTON_EDIT_MOT\n");
			switch (HIWORD(wParam)) {
			case EN_CHANGE:
				SendDlgItemMessage(hDlg, IDC_EDIT_POIDS, EM_SETREADONLY, TRUE, 0);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SUPPRIMER), false);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MODIFY), false);
				// Lecture du texte dans l'edit
				wchar_t buffer[MAX_WORD_LENGTH];
				GetDlgItemText(hDlg, IDC_EDIT_MOT, buffer, sizeof(buffer));
				FillLVDico(hDlg);
				if (wcslen(buffer) < 3) {
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_AJOUTER), false);
				}
				else {
					if (dictionnaire.ExisteMot(buffer)) {
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_AJOUTER), false);
					}
					else {
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_AJOUTER), true);
					}
				}
				break;
			}
		}
		return TRUE;
		case IDC_EDIT_POIDS: {
			switch (HIWORD(wParam)) {
			case EN_CHANGE: {
				wchar_t buffer[MAX_WORD_LENGTH];
				GetDlgItemText(hDlg, IDC_EDIT_MOT, buffer, sizeof(buffer));
				if (wcslen(buffer) > 0) 
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MODIFY), true);
				else 
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MODIFY), false);
				break;
			}
			}
		}
		return TRUE;
		case IDC_BUTTON_AJOUTER: { // Ajout d'un mot au dictionnaire
			if (debug) OutputDebugStringA("	IDC_BOUTON_AJOUTER\n");
			// Lecture du texte dans l'edit
			wchar_t buffer[MAX_WORD_LENGTH];
			GetDlgItemText(hDlg, IDC_EDIT_MOT, buffer, sizeof(buffer));
			// On verifie que le mot existe pas deja
			std::wstring message;
			dictionnaire.AjouterMot(buffer, 0);
			message = L"\"";
			message.append(buffer);
			message.append(GetXMLString(STR_DICO_NOTIF_ADDED).c_str());
			FillLVDico(hDlg);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_AJOUTER), false);
			// Mise a jour du message d'information
			SetDlgItemText(hDlg, IDC_STATIC_RETOUR, message.c_str());
		}
		return TRUE;
		case IDC_BUTTON_MODIFY: {
			int index = (int)SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_GETCURSEL, 0, 0);
			if (LB_ERR != index) {
				wchar_t buffer[MAX_WORD_LENGTH];
				int iSel = SendMessage(hListWord, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				ListView_GetItemText(hListWord, iSel, 0, buffer, MAX_WORD_LENGTH);
				wchar_t pondW[256];
				GetDlgItemText(hDlg, IDC_EDIT_POIDS, pondW, sizeof(pondW));
				unsigned int pond = _wtoi(pondW);
				dictionnaire.SetPond(buffer, pond);
				wstring message;
				message = L"\"";
				message.append(buffer);
				message.append(GetXMLString(STR_DICO_NOTIF_WORD_MODIFIED).c_str());

				SetDlgItemText(hDlg, IDC_STATIC_RETOUR, message.c_str());
				FillLVDico(hDlg);

				SetDlgItemText(hDlg, IDC_EDIT_POIDS, L"");
				SendDlgItemMessage(hDlg, IDC_EDIT_POIDS, EM_SETREADONLY, TRUE, 0);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MODIFY), false);

			}
		}
		return TRUE;
		case IDC_BUTTON_SUPPRIMER: { // Suppression d'un mot du dictionnaire
			if (debug) OutputDebugStringA("	IDC_BOUTON_SUPPRIMER\n");
			int index = (int)SendDlgItemMessage(hDlg, IDC_LIST_RESULTAT, LB_GETCURSEL, 0, 0);
			if (LB_ERR != index) {
				// Recuperation du mot
				wchar_t buffer[MAX_WORD_LENGTH];
				int iSel = SendMessage(hListWord, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				ListView_GetItemText(hListWord, iSel, 0, buffer, MAX_WORD_LENGTH);

				dictionnaire.SupprimerMot(buffer);

				FillLVDico(hDlg);
				GetDlgItemText(hDlg, IDC_EDIT_MOT, buffer, sizeof(buffer));
				FillLVDico(hDlg);
				if (wcslen(buffer) < 3) {
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_AJOUTER), false);
				}
				else {
					if (dictionnaire.ExisteMot(buffer)) {
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_AJOUTER), false);
					}
					else {
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_AJOUTER), true);
					}
				}
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SUPPRIMER), false);
				SetDlgItemText(hDlg, IDC_STATIC_RETOUR, GetXMLString(STR_DICO_NOTIF_WORD_DELETED).c_str());
			}
		}
		return TRUE;
		case IDC_CHECK_AUTO_INSERT:
			config.SetBoolParam(SPLASH_AUTO_INSERT, IsDlgButtonChecked(hDlg, IDC_CHECK_AUTO_INSERT) == BST_CHECKED);
			return TRUE;
		case IDOK: //Demande fermeture de la configuration
		case WM_DESTROY: 
		case WM_CLOSE:
			if (debug) OutputDebugStringA("	IDOK/WM_DESTROY/WM_CLOSE\n");
			dicoDlg = NULL;
			EndDialog(hDlg, 0);
			EnableCharHook(true);
			return TRUE;
		}
		break;
		default: break;
	}

	return FALSE;
}

LRESULT CALLBACK DlgSpeechProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	
	int test_lgt;

	switch (uMsg) {
	case WM_INITDIALOG: {// initialisation des champs du dialogue
		if(debug) OutputDebugStringA("SPEECH_INIT\n");
		SetWindowText(hDlg, GetXMLString(STR_TITLE_SPEECH).c_str());
		SetDlgItemText(hDlg, IDCANCEL, GetXMLString(STR_CANCEL).c_str());
		
		// Valeurs min/max pour le slide volume : 0 100
		SendDlgItemMessage(hDlg, IDC_SLIDER_VOICE_VOLUME, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)0);
		SendDlgItemMessage(hDlg, IDC_SLIDER_VOICE_VOLUME, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)100);

		// Valeurs min/max pour le slide vitesse : -10 10
		SendDlgItemMessage(hDlg, IDC_SLIDER_VOICE_SPEED, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)-10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_VOICE_SPEED, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)10);

		// Valeurs actuelles des slides
		SendDlgItemMessage(hDlg, IDC_SLIDER_VOICE_VOLUME, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)config.GetUnsignedIntParam(VOICE_VOLUME));
		SendDlgItemMessage(hDlg, IDC_SLIDER_VOICE_SPEED, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)config.GetRate());

		SendMessage(hDlg, WM_HSCROLL, NULL, NULL);

		std::deque<wstring> dequeVoices;
		GetVoices(dequeVoices);
		for (std::deque<wstring>::iterator iter = dequeVoices.begin();
			iter != dequeVoices.end();
			iter++) 
			SendDlgItemMessage(hDlg, IDC_COMBO_VOICE_SELECT, CB_ADDSTRING, 0, (LPARAM)iter->c_str());
		temp_voice_str = config.GetStringParam(VOICE_SELECTED_VOICE);
		int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_VOICE_SELECT, CB_FINDSTRINGEXACT, 0, (LPARAM)temp_voice_str.c_str());
		if (CB_ERR != index) {
			SendDlgItemMessage(hDlg, IDC_COMBO_VOICE_SELECT, CB_SETCURSEL, index, 0);
		}

		SendDlgItemMessage(hDlg, IDC_EDIT_VOICE_TEST, EM_SETCUEBANNER, FALSE, (LPARAM)GetXMLString(STR_SPEECH_TEST_CUE).c_str());

		CheckDlgButton(hDlg, IDC_CHECK_VOICE_HOVER, config.GetBoolParam(VOICE_ACTIVATION_HOVER) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_VOICE_INSERT, config.GetBoolParam(VOICE_ACTIVATION_INSERT) ? BST_CHECKED : BST_UNCHECKED);

		return TRUE;
	}
	case WM_HSCROLL: {
		if (debug) OutputDebugStringA("	WM_HSCROLL\n");
		unsigned int temp = SendDlgItemMessage(hDlg, IDC_SLIDER_VOICE_SPEED, TBM_GETPOS, 0, 0);
		wchar_t bufTemp[11];
		_itow_s(temp, bufTemp, 10);
		SendDlgItemMessage(hDlg, IDC_EDIT_VOICE_SPEED, WM_SETTEXT, 0, (LPARAM)bufTemp);
		pVoice->SetRate(temp);
		temp = SendDlgItemMessage(hDlg, IDC_SLIDER_VOICE_VOLUME, TBM_GETPOS, 0, 0);
		_itow_s(temp, bufTemp, 10);
		SendDlgItemMessage(hDlg, IDC_EDIT_VOICE_VOLUME, WM_SETTEXT, 0, (LPARAM)bufTemp);
		pVoice->SetVolume(temp);
		return TRUE;
	}
	case WM_COMMAND:
		if (debug) OutputDebugStringA("	WM_COMMAND\n");
		switch (LOWORD(wParam)) {
		case IDC_COMBO_VOICE_SELECT: {
			if (debug) OutputDebugStringA("	IDC_COMBO_VOICE_SELECT\n");
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_VOICE_SELECT, CB_GETCURSEL, 0, 0);
				if (CB_ERR != index) {
					wchar_t buf[MAX_WORD_LENGTH];
					SendDlgItemMessage(hDlg, IDC_COMBO_VOICE_SELECT, CB_GETLBTEXT, index, (LPARAM)buf);
					CComPtr<ISpObjectToken> v = GetVoiceFromString(buf);
					if (v != NULL) {
						HRESULT hr = pVoice->SetVoice(v);
						temp_voice_str = buf;
					}
				}
			}
			break;
		}
		case IDC_BUTTON_VOICE_TEST: {
			if (debug) OutputDebugStringA(" IDC_BUTTON_VOICE_TEST\n");
			test_lgt = SendDlgItemMessage(hDlg, IDC_EDIT_VOICE_TEST, WM_GETTEXTLENGTH, 0, 0);
			wchar_t buf[256];
			if (test_lgt > 0)
				GetDlgItemText(hDlg, IDC_EDIT_VOICE_TEST, buf, sizeof(buf));
			else
				SendDlgItemMessage(hDlg, IDC_EDIT_VOICE_TEST, EM_GETCUEBANNER,(WPARAM) buf, sizeof(buf));
			SayWord(buf);
			return TRUE;
		}
		case IDOK: {
			if (debug) OutputDebugStringA("	IDOK\n");
			config.SetStringParam(VOICE_SELECTED_VOICE, temp_voice_str);
			LONG rate;
			pVoice->GetRate(&rate);
			config.SetRate(rate);
			USHORT volume;
			pVoice->GetVolume(&volume);
			config.SetUnsignedIntParam(VOICE_VOLUME, volume);
			config.SetBoolParam(VOICE_ACTIVATION_HOVER, IsDlgButtonChecked(hDlg, IDC_CHECK_VOICE_HOVER) == BST_CHECKED);
			config.SetBoolParam(VOICE_ACTIVATION_INSERT, IsDlgButtonChecked(hDlg, IDC_CHECK_VOICE_INSERT) == BST_CHECKED);
			speechDlg = NULL;
			EndDialog(hDlg, 0);
			return TRUE;
		}
		case WM_DESTROY:
		case WM_CLOSE:
			if (debug) OutputDebugStringA("	WM_DESTROY/WM_CLOSE\n");
			LONG rate = config.GetRate();
			USHORT volume = config.GetUnsignedIntParam(VOICE_VOLUME);
			wstring v = config.GetStringParam(VOICE_SELECTED_VOICE);
			wchar_t v2[256];
			wcscpy(v2, v.c_str());
			pVoice->SetVoice(GetVoiceFromString(v2));
			pVoice->SetRate(rate);
			pVoice->SetVolume(volume);
			speechDlg = NULL;
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
	std::deque <std::wstring> * dequeFonts = (std::deque <std::wstring> *)(par);
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
	std::locale loc("French_France");
	
	// Variable utilisee pour simuler l'affichage d'un bouton
	static GestionnaireBoutons gestionnaireConfig;

	//gestionnaireConfig.SetNbBoutonsMax(1);

	switch(uMsg) {

	case WM_INITDIALOG: { // Initialisation des champs du dialogue
		if (debug) OutputDebugStringA("CONFIG_WM_INITDIALOG\n");
		EnableCharHook(false);
		SendMessage(mainDlg, PM_AFFICHERRECHERCHE, RESETWORD, 0);
		SetWindowText(hDlg, GetXMLString(STR_TITLE_CONFIG).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_COLORS, GetXMLString(STR_CONFIG_COLORS).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_STATE, GetXMLString(STR_CONFIG_COLORS_STATE).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_CONF_COLOR, GetXMLString(STR_CONFIG_COLORS_COLOR).c_str());
		SetDlgItemText(hDlg, IDC_RADIO_NORMAL, GetXMLString(STR_CONFIG_COLORS_STATE_NORMAL).c_str());
		SetDlgItemText(hDlg, IDC_RADIO_FOCUS, GetXMLString(STR_CONFIG_COLORS_STATE_SELECT).c_str());
		SetDlgItemText(hDlg, IDC_RADIO_TEXTE, GetXMLString(STR_CONFIG_COLORS_COLOR_TEXT).c_str());
		SetDlgItemText(hDlg, IDC_RADIO_FOND, GetXMLString(STR_CONFIG_COLORS_COLOR_BG).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_RED, GetXMLString(STR_CONFIG_COLORS_RED).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_GREEN, GetXMLString(STR_CONFIG_COLORS_GREEN).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_BLUE, GetXMLString(STR_CONFIG_COLORS_BLUE).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_OVERVIEW, GetXMLString(STR_CONFIG_OVERVIEW).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_FONT, GetXMLString(STR_CONFIG_POLICE).c_str());
		SetDlgItemText(hDlg, IDC_CHECK_GRAS, GetXMLString(STR_CONFIG_POLICE_BOLD).c_str());
		SetDlgItemText(hDlg, IDC_CHECK_ITALIQUE, GetXMLString(STR_CONFIG_POLICE_ITALIC).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_ACCESS, GetXMLString(STR_CONFIG_ACCESS).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_NBLETTER, GetXMLString(STR_CONFIG_ACCESS_NBLETTER).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_TIME, GetXMLString(STR_CONFIG_ACCESS_TIMETOREDUCE).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_NBWORD, GetXMLString(STR_CONFIG_ACCESS_NBWORD).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_TRANSPARENCY, GetXMLString(STR_CONFIG_ACCESS_TRANSPARENCY).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_TRANSPARENT, GetXMLString(STR_CONFIG_ACCESS_TRANSPARENT).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_OPAC, GetXMLString(STR_CONFIG_ACCESS_OPAC).c_str());
		SetDlgItemText(hDlg, IDC_BUTTON_SPEECH_SYNTHESIS, GetXMLString(STR_CONFIG_ACCESS_SPEECH).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_GROUP_SELECT, GetXMLString(STR_CONFIG_SELECT).c_str());
		SetDlgItemText(hDlg, IDC_CHECK_SELECT_BY_FUNCTION, GetXMLString(STR_CONFIG_SELECT_FUNCTION).c_str());
		SetDlgItemText(hDlg, IDC_CHECK_SELECT_BY_CLIC, GetXMLString(STR_CONFIG_SELECT_CLIC).c_str());
		SetDlgItemText(hDlg, IDC_CHECK_SELECT_BY_ARROWS, GetXMLString(STR_CONFIG_SELECT_ARROWS).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_GROUP_SUPPORT, GetXMLString(STR_CONFIG_SUPPORT).c_str());
		wstring message = L"";
		message.append(GetXMLString(STR_CONFIG_SUPPORT_HELP).c_str());
		//message.append(L"\n");
		//message.append(GetXMLString(STR_CONFIG_SUPPORT_HELP_2).c_str());
		SetDlgItemText(hDlg, IDC_STATIC_SUPPORT_HELP, message.c_str());
		SetDlgItemText(hDlg, IDC_STATIC_SUPPORT_MAIL, GetXMLString(STR_CONFIG_SUPPORT_MAIL).c_str());
		SetDlgItemText(hDlg, IDCANCEL, GetXMLString(STR_CANCEL).c_str());

		// Initialisation du gestionnaire d'apercu
		gestionnaireConfig.SetDialogue(hDlg);
		gestionnaireConfig.SetIdBase(IDC_BUTTON_APERCU);
		gestionnaireConfig.SetCouleurText(gestionnaireBoutons.GetCouleurText());
		gestionnaireConfig.SetCouleurBk(gestionnaireBoutons.GetCouleurBk());
		gestionnaireConfig.SetCouleurFocusText(gestionnaireBoutons.GetCouleurFocusText());
		gestionnaireConfig.SetCouleurFocusBk(gestionnaireBoutons.GetCouleurFocusBk());
		gestionnaireConfig.SetNbBoutons(1);
		gestionnaireConfig.SetPolice(gestionnaireBoutons.GetPolice());
		gestionnaireConfig.SetPoliceBold(gestionnaireBoutons.GetPoliceBold());
		gestionnaireConfig.SetPoliceItalic(gestionnaireBoutons.GetPoliceItalic());
		gestionnaireConfig.SetBoutonText(0, L"Exemple");
		gestionnaireConfig.AppliquerFont(false);
		// valeur pour les slides barres max
		SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)255);
		SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)255);
		SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)255);
		SendDlgItemMessage(hDlg, IDC_SLIDER_NBWORD, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_OPACITY, TBM_SETRANGEMAX, (WPARAM)(BOOL)true, (LPARAM)(LONG)255);
		// valeur pour les slides barres min
		SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)0);
		SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)0);
		SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)0);
		SendDlgItemMessage(hDlg, IDC_SLIDER_NBWORD, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)3);
		SendDlgItemMessage(hDlg, IDC_SLIDER_OPACITY, TBM_SETRANGEMIN, (WPARAM)(BOOL)true, (LPARAM)(LONG)127);

		COLORREF couleur;
		couleur = RGB(SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_GETPOS, 0, 0)
			, SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_GETPOS, 0, 0)
			, SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_GETPOS, 0, 0));
		// Selection bouton texte
		CheckDlgButton(hDlg, IDC_CHECK_GRAS, gestionnaireConfig.GetPoliceBold()?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_ITALIQUE, gestionnaireConfig.GetPoliceItalic()?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_RADIO_NORMAL, BST_CHECKED);
		CheckDlgButton(hDlg, IDC_RADIO_TEXTE, BST_CHECKED);
		SendMessage(hDlg, WM_COMMAND, IDC_RADIO_NORMAL, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_RADIO_TEXTE, 0);
		// Enumeration des polices d'ecriture
		HDC hDC = GetDC(hDlg);
		std::deque <std::wstring> dequeFonts;
		EnumFontFamilies(hDC, NULL, (FONTENUMPROC)callbackfunc, (LPARAM)&dequeFonts);
		std::sort(dequeFonts.begin(),dequeFonts.end());
		ReleaseDC(hDlg, hDC);
		// Affichage dans le combox box
		for (std::deque<std::wstring>::iterator iter = dequeFonts.begin();
			iter!=dequeFonts.end();
			++iter) {
			SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_ADDSTRING, 0, (LPARAM)iter->c_str());
			//MessageBox(NULL, (*iter).c_str(), "Font", MB_OK);
		}
		// Selection de la police courante
		int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_FINDSTRINGEXACT, 0, (LPARAM)gestionnaireConfig.GetPolice().c_str());
		if (CB_ERR != index) {
			SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_SETCURSEL, index, 0);
		}
		wchar_t bufferConv[11];
		// Affichage du nombre de lettres avant affichage
		_ultow_s(config.GetUnsignedIntParam(SPLASH_NB_LETTERS), bufferConv, 10);
		SetDlgItemText(hDlg, IDC_EDIT_NB_LETTRE, bufferConv);
		// Affichage du nombre de secondes avant cachage...
		_ultow_s(config.GetUnsignedIntParam(SPLASH_TIME) / 1000, bufferConv, 10);
		SetDlgItemText(hDlg, IDC_EDIT_TEMPS, bufferConv);
		
		// Affichage du checkbox de synthèse vocale
		CheckDlgButton(hDlg, IDC_CHECK_SYNTHESIS, config.GetBoolParam(VOICE_ACTIVATION_HOVER) ? BST_CHECKED : BST_UNCHECKED);
		// Affichage du nombre de prédictions
		unsigned int nbword = config.GetUnsignedIntParam(NB_WORD);
		SendDlgItemMessage(hDlg, IDC_SLIDER_NBWORD, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)nbword);
		_ultow_s(nbword, bufferConv, 10);
		SetDlgItemText(hDlg, IDC_EDIT_NBWORD, bufferConv);
		// Réglage slide opacité
		unsigned int alpha = config.GetUnsignedIntParam(DIALOG_OPACITY);
		SendDlgItemMessage(hDlg, IDC_SLIDER_OPACITY, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)alpha);
		CheckDlgButton(hDlg, IDC_CHECK_SELECT_BY_ARROWS, config.GetBoolParam(SELECT_BY_ARROWS) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_SELECT_BY_FUNCTION, config.GetBoolParam(SELECT_BY_FUNCTION) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_SELECT_BY_CLIC, config.GetBoolParam(SELECT_BY_CLIC) ? BST_CHECKED : BST_UNCHECKED);
		}
		return TRUE;

	case WM_DRAWITEM: { //Dessin des boutons
		if (debug) OutputDebugStringA("CONFIG_WM_DRAWITEM\n");
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
		if (debug) OutputDebugStringA("CONFIG_WM_HSCROLL\n");
		// Creation de la couleur RGB issue du mouvement
		COLORREF couleur;
		couleur = RGB(SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_GETPOS, 0, 0)
			, SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_GETPOS, 0, 0)
			, SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_GETPOS, 0, 0));
		// Valeur dans les textes box
		wchar_t bufferConv[11];
		_itow_s(GetRValue(couleur), bufferConv, 10);
		SendDlgItemMessage(hDlg, IDC_EDIT_ROUGE, WM_SETTEXT, NULL, (LPARAM)bufferConv);
		_itow_s(GetGValue(couleur), bufferConv, 10);
		SendDlgItemMessage(hDlg, IDC_EDIT_VERT, WM_SETTEXT, NULL, (LPARAM)bufferConv);
		_itow_s(GetBValue(couleur), bufferConv, 10);
		SendDlgItemMessage(hDlg, IDC_EDIT_BLEU, WM_SETTEXT, NULL, (LPARAM)bufferConv);
		
		// En fonction du checkbox on modifie les parametres associes
		if (IsDlgButtonChecked(hDlg, IDC_RADIO_NORMAL) == BST_CHECKED) {
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_TEXTE) == BST_CHECKED)
				gestionnaireConfig.SetCouleurText(couleur);
			else
				gestionnaireConfig.SetCouleurBk(couleur);
		}
		else {
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_TEXTE) == BST_CHECKED)
				gestionnaireConfig.SetCouleurFocusText(couleur);
			else
				gestionnaireConfig.SetCouleurFocusBk(couleur);
		}
		// MAJ du bouton
		SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, true, 0);
		SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, false, 0);

		unsigned int nb_max = SendDlgItemMessage(hDlg, IDC_SLIDER_NBWORD, TBM_GETPOS, 0, 0);
		_itow_s(nb_max, bufferConv, 10);
		SendDlgItemMessage(hDlg, IDC_EDIT_NBWORD, WM_SETTEXT, NULL, (LPARAM)bufferConv);
		}
		break;

	case WM_COMMAND: //Message de type command
		if (debug) OutputDebugStringA("CONFIG_WM_COMMAND\n");
		switch (LOWORD(wParam)) {
		case IDC_COMBO_POLICE:
			if (debug) OutputDebugStringA("	IDC_COMBO_POLICE\n");
			if (HIWORD(wParam) == CBN_SELCHANGE) {  //Changement d'onglet pour la police
				// Lecture de la police courante
				int index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_GETCURSEL, 0, 0);
				if (CB_ERR != index) {
					wchar_t buffer[MAX_WORD_LENGTH];
					SendDlgItemMessage(hDlg, IDC_COMBO_POLICE, CB_GETLBTEXT, index, (LPARAM)(LPTSTR)buffer);
					gestionnaireConfig.SetPolice(buffer);
					gestionnaireConfig.AppliquerFont();
					SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, true, 0);
					SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, false, 0);
				}
				return TRUE;
			}
			break;

		case IDC_RADIO_FOCUS:
		case IDC_RADIO_NORMAL:
		case IDC_RADIO_TEXTE:
		case IDC_RADIO_FOND: {
			if (debug) OutputDebugStringA("	IDC_RADIO\n");
			COLORREF couleur;
			// En fonction du checkbox on lit les parametres associes
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_NORMAL) == BST_CHECKED) {
				gestionnaireConfig.SetBoutonFocus(-1);
				if (IsDlgButtonChecked(hDlg, IDC_RADIO_TEXTE) == BST_CHECKED) {
					couleur = gestionnaireConfig.GetCouleurText();
				}
				else {
					couleur = gestionnaireConfig.GetCouleurBk();
				}
			}
			else {
				gestionnaireConfig.SetBoutonFocus(0);
				if (IsDlgButtonChecked(hDlg, IDC_RADIO_TEXTE) == BST_CHECKED) {
					couleur = gestionnaireConfig.GetCouleurFocusText();
				}
				else {
					couleur = gestionnaireConfig.GetCouleurFocusBk();
				}
			}
			// valeur pour la couleur
			SendDlgItemMessage(hDlg, IDC_SLIDER_ROUGE, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)GetRValue(couleur));
			SendDlgItemMessage(hDlg, IDC_SLIDER_VERT, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)GetGValue(couleur));
			SendDlgItemMessage(hDlg, IDC_SLIDER_BLEU, TBM_SETPOS, (WPARAM)(BOOL)true, (LPARAM)(LONG)GetBValue(couleur));
			// Valeur dans les textes box
			char bufferConv[11];
			_itoa_s(GetRValue(couleur), bufferConv, 10);
			SendDlgItemMessage(hDlg, IDC_EDIT_ROUGE, WM_SETTEXT, NULL, (LPARAM)Utf8ToUtf16(bufferConv).c_str());
			_itoa_s(GetGValue(couleur), bufferConv, 10);
			SendDlgItemMessage(hDlg, IDC_EDIT_VERT, WM_SETTEXT, NULL, (LPARAM)Utf8ToUtf16(bufferConv).c_str());
			_itoa_s(GetBValue(couleur), bufferConv, 10);
			SendDlgItemMessage(hDlg, IDC_EDIT_BLEU, WM_SETTEXT, NULL, (LPARAM)Utf8ToUtf16(bufferConv).c_str());


			SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, true, 0);
			SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, false, 0);
		}
		break;

		case IDC_CHECK_SYNTHESIS: 
			break;

		case IDC_CHECK_GRAS: // Selection du type de police
		case IDC_CHECK_ITALIQUE: {
			if (debug) OutputDebugStringA("	IDC_CHECK\n");
			gestionnaireConfig.SetPoliceBold(IsDlgButtonChecked(hDlg, IDC_CHECK_GRAS) == BST_CHECKED);
			gestionnaireConfig.SetPoliceItalic(IsDlgButtonChecked(hDlg, IDC_CHECK_ITALIQUE) == BST_CHECKED);
			gestionnaireConfig.AppliquerFont(); // Maj de la font
			SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, true, 0);
			SendDlgItemMessage(hDlg, IDC_BUTTON_APERCU, BM_SETSTATE, false, 0);
			}
			break;

		case IDC_BUTTON_SPEECH_SYNTHESIS: {
			if (speechDlg == NULL) {
				speechDlg = CreateDialog(hInst,
					MAKEINTRESOURCE(IDD_DIALOG_SPEECH_SYNTHESIS),
					hDlg,
					(DLGPROC)DlgSpeechProc);
				ShowWindow(speechDlg, WS_VISIBLE);
			}
			return TRUE;
		}

		case IDOK: { //Demande fermeture de la configuration
			if (debug) OutputDebugStringA("	IDOK\n");
			if (IsDlgButtonChecked(hDlg, IDC_CHECK_SELECT_BY_FUNCTION) != BST_CHECKED
				&& IsDlgButtonChecked(hDlg, IDC_CHECK_SELECT_BY_ARROWS) != BST_CHECKED
				&& IsDlgButtonChecked(hDlg, IDC_CHECK_SELECT_BY_CLIC) != BST_CHECKED) {
				wchar_t msg[256];
				wsprintf(msg, L"%s\n%s\n%s",
					GetXMLString(STR_ERROR_SELECT_1),
					GetXMLString(STR_ERROR_SELECT_2),
					GetXMLString(STR_ERROR_SELECT_3));
				MessageBox(hDlg, msg, GetXMLString(STR_ERROR_SELECT_TITLE).c_str(), MB_ICONERROR | MB_OK | MB_TASKMODAL);
				return TRUE;
			}
			else {
				// Sauvegarde de la configuration de l'apercu pour l'application
				gestionnaireBoutons.SetCouleurText(gestionnaireConfig.GetCouleurText());
				gestionnaireBoutons.SetCouleurBk(gestionnaireConfig.GetCouleurBk());
				gestionnaireBoutons.SetCouleurFocusText(gestionnaireConfig.GetCouleurFocusText());
				gestionnaireBoutons.SetCouleurFocusBk(gestionnaireConfig.GetCouleurFocusBk());
				gestionnaireBoutons.SetPolice(gestionnaireConfig.GetPolice());
				gestionnaireBoutons.SetPoliceBold(gestionnaireConfig.GetPoliceBold());
				gestionnaireBoutons.SetPoliceItalic(gestionnaireConfig.GetPoliceItalic());
				gestionnaireBoutons.AppliquerFont();
				// Lecture du nombre de lettres avant affichage
				wchar_t bufferRead[11];
				GetDlgItemText(hDlg, IDC_EDIT_NB_LETTRE, bufferRead, sizeof(bufferRead));
				config.SetUnsignedIntParam(SPLASH_NB_LETTERS, _wtol(bufferRead));
				// Lecture du nombre de seconde avant cachage...
				GetDlgItemText(hDlg, IDC_EDIT_TEMPS, bufferRead, sizeof(bufferRead));
				config.SetUnsignedIntParam(SPLASH_TIME, _wtol(bufferRead) * 1000);
				// Lecture du % d'opacité
				unsigned int alpha = SendDlgItemMessage(hDlg, IDC_SLIDER_OPACITY, TBM_GETPOS, 0, 0);
				SetLayeredWindowAttributes(mainDlg, NULL, alpha, LWA_ALPHA);
				config.SetUnsignedIntParam(DIALOG_OPACITY, alpha);

				unsigned int nb_word = SendDlgItemMessage(hDlg, IDC_SLIDER_NBWORD, TBM_GETPOS, 0, 0);
				gestionnaireBoutons.SetNbWord(nb_word);
				config.SetUnsignedIntParam(NB_WORD, nb_word);
				// Affichage du checkbox d'auto insertion
				config.SetBoolParam(SELECT_BY_ARROWS, IsDlgButtonChecked(hDlg, IDC_CHECK_SELECT_BY_ARROWS) == BST_CHECKED);
				config.SetBoolParam(SELECT_BY_FUNCTION, IsDlgButtonChecked(hDlg, IDC_CHECK_SELECT_BY_FUNCTION) == BST_CHECKED);
				config.SetBoolParam(SELECT_BY_CLIC, IsDlgButtonChecked(hDlg, IDC_CHECK_SELECT_BY_CLIC) == BST_CHECKED);
				// checkbox speech synthesis
				//config.SetBoolParam(VOICE_ACTIVATION_HOVER, IsDlgButtonChecked(hDlg, IDC_CHECK_SYNTHESIS) == BST_CHECKED);
				EnableSelectByArrows(config.GetBoolParam(SELECT_BY_ARROWS));
				EnableSelectByFunction(config.GetBoolParam(SELECT_BY_FUNCTION));
				EnableCharHook(true);
			}
		}
		case WM_DESTROY:
		case WM_CLOSE: {
			if (debug) OutputDebugStringA("	WM_DESTROY/WM_CLOSE\n");
			configDlg = NULL;
			EndDialog(hDlg, 0);
			EnableCharHook(true);
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
		if (debug) OutputDebugStringA("INSERT_INITDIALOG\n");
		// Timer avant fermeture
		SetTimer(hDlg, IDT_TIMER_HIDE, config.GetUnsignedIntParam(SPLASH_TIME), (TIMERPROC)NULL);
		SetDlgItemText(hDlg, IDC_STATIC_INSERER, currentSearch.c_str());
		return TRUE;

	case WM_TIMER: // Reception d'un evenement timer 
		if (debug) OutputDebugStringA("INSERT_WM_TIMER\n");
		switch (wParam) { 
		case IDT_TIMER_HIDE: // Disparision de la fenetre 
			KillTimer(hDlg, IDT_TIMER_HIDE);
			insertDlg = NULL;
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;

	case WM_COMMAND: //Message de type command
		if (debug) OutputDebugStringA("INSERT_WM_COMMAND\n");
		switch (LOWORD(wParam)) {
		case IDOK: { //Demande fermeture de la configuration
			if (debug) OutputDebugStringA("	IDOK\n");
			wchar_t bufferRead[MAX_WORD_LENGTH];
			GetDlgItemText(hDlg, IDC_STATIC_INSERER, bufferRead, sizeof(bufferRead) -1);
			// Ajout du mot au dictionnaire
			if (!dictionnaire.ExisteMot(bufferRead))
				dictionnaire.AjouterMot(bufferRead);
			}
		case WM_DESTROY:
		case WM_CLOSE:
			if (debug) OutputDebugStringA("	WM_DESTROY/WM_CLOSE\n");
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

	switch (uMsg) {
	case WM_INITDIALOG: //Initialisation du dialogue
		if (debug) OutputDebugStringA("MAIN_WM_INITDIALOG\n");
		return TRUE;

	case WM_TIMER: // Reception d'un evenement timer 
		if (debug) OutputDebugStringA("MAIN_WM_TIMER\n");
		switch (wParam) {
		case IDT_TIMER_HIDE: // Disparision de la fenetre principale
			if (debug) OutputDebugStringA("	IDT_TIMER_HIDE\n");
			// On cache la fenetre principale
			ShowWindow(mainDlg, SW_HIDE);
			EnableEnterHook(true);
			// Destruction du timer de hide
			KillTimer(hDlg, IDT_TIMER_HIDE);
			break;
		case IDT_TIMER_SAY:
			if (debug) OutputDebugStringA("	IDT_TIMER_SAY\n");
			if (config.GetBoolParam(VOICE_ACTIVATION_HOVER)) {
				SayWord(gestionnaireBoutons.GetBoutonText(gestionnaireBoutons.GetBoutonFocus(), L"").c_str());
				KillTimer(hDlg, IDT_TIMER_SAY);
			}
			break;
		}
		break;

	case WM_DRAWITEM: { //Dessin des boutons
		//if (debug) OutputDebugString("MAIN_WM_DRAWITEM\n");
		// Structure d'information pour le dessin
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
		if ((lpdis->CtlID >= IDC_BUTTON_F1) && (lpdis->CtlID<(IDC_BUTTON_F1 + NB_MAX_BUTTONS))) {
			// Bouton de l'interface
			gestionnaireBoutons.DessinerBouton(lpdis, lpdis->CtlID - IDC_BUTTON_F1);
			return TRUE;
		}
		break;
	}

	case WM_COMMAND: //Message de type command
		if (debug) OutputDebugStringA("MAIN_WM_COMMAND\n");
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
			if (debug) OutputDebugStringA("	IDC_BUTTON_FX\n");
			// N'autorise que les bouttons affiches
			if (LOWORD(wParam) - IDC_BUTTON_F1 < (int)gestionnaireBoutons.GetNbBoutons() && active) {
				if (config.GetBoolParam(SELECT_BY_CLIC) || lParam == 161L) {
					WriteWord(LOWORD(wParam) - IDC_BUTTON_F1); 
				}
				else if (lParam != 161L)
					SetForegroundWindow(editor);
			}
			return TRUE;
		case WM_DESTROY: //Demande fermeture de l'application
			if (debug) OutputDebugStringA("	WM_DESTROY\n");
		case WM_CLOSE:
			if (debug) OutputDebugStringA("	WM_CLOSE\n");
			// On cache la fenetre principale
			ShowWindow(mainDlg, SW_HIDE);
			return TRUE;
		case RACCOURCI_DISABLE:
			char str_debug[256];
			sprintf_s(str_debug, "	RACCOURCI DISABLE %s -> %s\n", active ? "true" : "false", active ? "false" : "true");
			if(debug) OutputDebugStringA(str_debug);
			active = !active;
			LoadTaskBarStr();
			SetButtonFocus(gestionnaireBoutons.GetBoutonFocus());
			return TRUE;
		default:
			switch ((UINT)lParam) {

			case WM_RBUTTONDOWN: { //Affichage d'un popup menu
				if (debug) OutputDebugStringA("	WM_RBUTTONDOWN\n");
				//Recuperation de la position de la souris
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hDlg);
				//Affichage du menu et attente d'un choix
				int i = (int)TrackPopupMenu(mainDlgSubMenu, TPM_RETURNCMD | TPM_LEFTBUTTON, pt.x, pt.y, 0, hDlg, 0);
				switch (i) {
				case ID_POPUP_CONFIGURER: // Affichage de la fenetre de configuration
					if (NULL == configDlg) {
						configDlg = CreateDialog(hInst
							, MAKEINTRESOURCE(IDD_DIALOG_CONFIGURER)
							, mainDlg
							, (DLGPROC)DlgConfigProc);
					}
					return TRUE;
				case ID_POPUP_DICTIONNAIRE: // Affichage de la fenetre de dictionnaire
					if (debug) OutputDebugStringA("	ID_POPUP_DICTIONNAIRE\n");
					if (NULL == dicoDlg) {
						dicoDlg = CreateDialog(hInst
							, MAKEINTRESOURCE(IDD_DIALOG_DICTIONNAIRE)
							, mainDlg
							, (DLGPROC)DlgDicoProc);
					}
					return TRUE;
				case ID_POPUP_QUITTER: // On quitte l'application
					if (debug) OutputDebugStringA("	ID_POPUP_QUITTER\n");
					QuitDicom();
					return TRUE;
				case ID_POPUP_DISABLE:
					SendMessage(hDlg, WM_COMMAND, RACCOURCI_DISABLE, NULL);
					return TRUE;
				}
				break;
			}
			}
			break;
		}
		break;

	case PM_AUTOINSERT: // Gere la validation d'une insertion automatique
		if (debug) OutputDebugStringA("MAIN_PM_AUTOINSERT\n");
		switch (wParam) {
		case INSERTWORD: // Ajoute le mot en attente d'insertion (cf. dialog)
			if (debug) OutputDebugStringA("	INSERTWORD\n");
			if (NULL != insertDlg && active)
				SendMessage(insertDlg, WM_COMMAND, IDOK, 0L);
			return TRUE;
		}
		return TRUE;

	case PM_AFFICHERRECHERCHE: //Demande issu d'une saisie clavier (DLLHOOK)
		if (debug) OutputDebugStringA("MAIN_PM_AFFICHERRECHERCHE\n");
		switch (wParam) {
		case GOUP:
			if (debug) OutputDebugStringA("	VK_UP\n");
			if (active) {
				if (gestionnaireBoutons.GetBoutonFocus() == -1 || gestionnaireBoutons.GetBoutonFocus() == 0) {
					SetButtonFocus(gestionnaireBoutons.GetNbBoutons() - 1);
				}
				else if (gestionnaireBoutons.GetBoutonFocus() > 0) {
					SetButtonFocus(gestionnaireBoutons.GetBoutonFocus() - 1);
				}
				return TRUE;
			}
		case GODOWN:
			if (debug) OutputDebugStringA("	VK_DOWN\n");
			if (active) {
				if (gestionnaireBoutons.GetBoutonFocus() == -1
					|| gestionnaireBoutons.GetBoutonFocus() == gestionnaireBoutons.GetNbBoutons() - 1) {
					SetButtonFocus(0);
				}
				else if (gestionnaireBoutons.GetBoutonFocus() < gestionnaireBoutons.GetNbBoutons() - 1) {
					SetButtonFocus(gestionnaireBoutons.GetBoutonFocus() + 1);
				}
				return TRUE;
			}
		case VALIDATEFOCUS:
			if (debug) OutputDebugStringA("	VALIDATE_FOCUS\n");
			if (active) {
				SendMessage(mainDlg, WM_COMMAND, IDC_BUTTON_F1 + gestionnaireBoutons.GetBoutonFocus(), 0L);
				return TRUE;
			}
		case RESETWORD: // Met a zero le buffer de recherche
			if (debug) OutputDebugStringA("	RESETWORD\n");
			// Affichage d'un dialogue pour l'insertion du mot dans le dico
			if (config.GetBoolParam(SPLASH_AUTO_INSERT) && (currentSearch.length() >= 3) && active) {

				// Traitement des caracteres pourris en fin de recherche
				switch (currentSearch[currentSearch.length() - 1]) {
				case '.': // Retrait du dernier caractere
					currentSearch.resize(currentSearch.length() - 1);
					break;
				}

				// Si le mot n'existe pas dans le dictionnaire
				if ((NULL == insertDlg) && (dictionnaire.ExisteMot(currentSearch) == false) ) {
					HWND before = GetForegroundWindow();
					insertDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_INSERTION), hDlg, (DLGPROC)DlgInsertProc);
					SetForegroundWindow(insertDlg);
					// Lecture des coordonnees du dialogue
					RECT dlgRect;
					GetWindowRect(insertDlg, &dlgRect);

					// Positionnement du dialogue en bas a droite
					MoveWindow(insertDlg
						, GetSystemMetrics(SM_CXSCREEN) - (dlgRect.right - dlgRect.left)
						, GetSystemMetrics(SM_CYSCREEN) - (dlgRect.bottom - dlgRect.top + 35)
						, dlgRect.right - dlgRect.left
						, dlgRect.bottom - dlgRect.top
						, TRUE);

					// Remise en place de la fenetre d'avant
					SetForegroundWindow(before);
				}
			}
			// Reset de la recherche courante
			currentSearch.clear();
			// On raffraichit la fenetre d'affichage
			gestionnaireBoutons.SetNbBoutons(0);
			SetButtonFocus(-1);
			return TRUE;

		case APPENDCHAR: {// Ajoute un caractere au mot de recherche
				if (NULL != insertDlg) {
					DestroyWindow(insertDlg);
					insertDlg = NULL;
				}
				wchar_t buffer[2];
				memset(buffer, 0, sizeof(buffer));
				buffer[0] = (wchar_t)lParam;
				if (buffer[0] == ' ') {
					SendMessage(mainDlg, PM_AFFICHERRECHERCHE, RESETWORD, 0L);
					return TRUE;
				}
			
				// Recomposition de la lettre recue
				wchar_t str_debug[256];
				wsprintf(str_debug, L"	APPENDCHAR %c\n", buffer[0]);
				if (debug) OutputDebugString(str_debug);
				// Destruction du dialogue d'insertion
				if (NULL != insertDlg) {
					DestroyWindow(insertDlg);
					insertDlg = NULL;
				}
				/*UGO MODIF*/
				if (editor == NULL || GetForegroundWindow() != editor)
					editor = GetForegroundWindow();

				// Si le caractere est particulier
				switch (buffer[0]) {
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
				case ' ':
					SendMessage(hDlg, PM_AFFICHERRECHERCHE, (WPARAM)RESETWORD, 0L);
					return TRUE;
				}

				currentSearch.append(buffer);

				// Lancement de la recherche
				RechercherDictionnaire();
			
			return TRUE;
		}

		case DELETELASTCHAR: // Supprime le dernier caractere de la recherche
			if (debug) OutputDebugStringA("	DELETELASTCHAR\n");
			if (currentSearch.length() > 0) {
				// Retrait du dernier caractere
				currentSearch.resize(currentSearch.length() - 1);

				// Lancement de la recherche
				RechercherDictionnaire();
			}
			return TRUE;
		}
		break;

	case WM_SIZING: { //Sur une action de redimensionnement
		if (debug) OutputDebugStringA("MAIN_WM_SIZING\n");
		// Lecture des coordonnees du dialogue
		RECT dlgRect;
		GetWindowRect(hDlg, &dlgRect);

		// Lecture des dimension de la zone client
		RECT dlgClientRect;
		GetClientRect(hDlg, &dlgClientRect);

		//Hauteur minimum atteinte
		int hMin = ((dlgRect.bottom - dlgRect.top) - dlgClientRect.bottom) + 20 + (gestionnaireBoutons.GetNbBoutons() == 0 ? 1 : gestionnaireBoutons.GetNbBoutons())*heightButtonDefault;
		if ((((RECT*)lParam)->bottom - ((RECT*)lParam)->top) < hMin) {
			((RECT*)lParam)->bottom = ((RECT*)lParam)->top + (long)(hMin);
		}

		//Largueur minimal atteinte
		if ((((RECT*)lParam)->right - ((RECT*)lParam)->left) < MAIN_DLG_DEFAULT_WIDTH) {
			((RECT*)lParam)->right = ((RECT*)lParam)->left + MAIN_DLG_DEFAULT_WIDTH;
		}
		return TRUE;
	}

	case WM_SIZE: // Action aprés redimensionnement
		if (debug) OutputDebugStringA("MAIN_WM_SIZE\n");
		gestionnaireBoutons.PositionnerBoutons();
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK TheButtonSubclass(HWND hWnd, UINT uMsg, WPARAM wParam,
	LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	BOOL tracking = false;
	switch (uMsg) {
	case WM_MOUSEMOVE:
		if (!tracking) {
			tracking = true;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.hwndTrack = GetDlgItem((HWND)dwRefData, uIdSubclass + IDC_BUTTON_F1);
			tme.dwHoverTime = 0;
			TrackMouseEvent(&tme);
			break;
		}
	case WM_MOUSEHOVER:
		if (config.GetBoolParam(VOICE_ACTIVATION_HOVER)) {
			SayWord(gestionnaireBoutons.GetBoutonText(uIdSubclass, L"").c_str());
			SetTimer(mainDlg, IDT_TIMER_HIDE, config.GetUnsignedIntParam(SPLASH_TIME), (TIMERPROC)NULL);
			tracking = false;
			break;
		}
	case WM_MOUSELEAVE:
		tracking = false;
		break;
	default:
		break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}


/*
 * @brief Point d'entree de l'application windows DICOM.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{   
	INITCOMMONCONTROLSEX icex;
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);
	/**************************MUTEX*************************/
	/**************(to have only one instance)***************/
	mutex = OpenMutex(MUTEX_ALL_ACCESS, 0, L"DICOM");
	if (!mutex) 
		mutex = CreateMutex(0, 0, L"DICOM");
	else
		return 0;


	std::locale("");

	MoveToExeDirectory();

	config = Configuration(CONFIG_FILE_NAME);

	/*********************VARIABLES GLOBALES*****************/
	hInst = hInstance;

	/*******************SPEECH SYNTHESIS*********************/

	HRESULT result = InitSpeech();

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
					EnableEnterHook = (DLLEnableEnterHook)GetProcAddress(hinstDLLHook, "EnableEnterHook");
					if (NULL != EnableEnterHook) {
						EnableCharHook = (DLLEnableCharHook)GetProcAddress(hinstDLLHook, "EnableCharHook");
						if (NULL != EnableCharHook) {
							EnableSelectByFunction = (DLLEnableSelectByFunction)GetProcAddress(hinstDLLHook, "EnableSelectByFunction");
							if (NULL != EnableSelectByFunction) {
								EnableSelectByArrows = (DLLEnableSelectByArrows)GetProcAddress(hinstDLLHook, "EnableSelectByArrows");
								if (NULL != EnableSelectByArrows) {
									EnableOtherOption = (DLLEnableOtherOption)GetProcAddress(hinstDLLHook, "EnableOtherOption");
									if (NULL != EnableOtherOption)
										loaded = true;
								}
							}
						}
					}
				}
			}
		}

		if (false == loaded) {
			wchar_t error[256];
			wsprintf(error, L"%s%s%s\n%s", 
				GetXMLString(STR_ERROR_HOOK_1).c_str(), 
				HOOKDLL_FILE_NAME,
				GetXMLString(STR_ERROR_HOOK_2).c_str(),
				GetXMLString(STR_ERROR_HOOK_3).c_str());
			MessageBox(NULL
				, error
				, GetXMLString(STR_ERROR_HOOK_TITLE).c_str()
				, MB_ICONERROR | MB_OK | MB_TASKMODAL);
			return 1;
		}
	}

	EnableSelectByFunction(config.GetBoolParam(SELECT_BY_FUNCTION));
	EnableSelectByArrows(config.GetBoolParam(SELECT_BY_ARROWS));

	/*********************CONFIGURATION***********************/
	gestionnaireBoutons.SetNbWord(config.GetUnsignedIntParam(NB_WORD));
	if (wcscmp(config.GetStringParam(FONT_TYPE).c_str(), L"NULL") != 0) {
		wchar_t police[256];
		wcscpy(police, config.GetStringParam(FONT_TYPE).c_str());
		gestionnaireBoutons.SetPolice(police);
		gestionnaireBoutons.SetPoliceBold(config.GetBoolParam(FONT_BOLD));
		gestionnaireBoutons.SetPoliceItalic(config.GetBoolParam(FONT_ITALIC));
		gestionnaireBoutons.SetCouleurText(config.GetUnsignedIntParam(FONT_TEXT_COLOR));
		gestionnaireBoutons.SetCouleurFocusText(config.GetUnsignedIntParam(FONT_TEXT_FOCUS_COLOR));
		gestionnaireBoutons.SetCouleurBk(config.GetUnsignedIntParam(DIALOG_BACKGROUND_COLOR));
		gestionnaireBoutons.SetCouleurFocusBk(config.GetUnsignedIntParam(DIALOG_BACKGROUND_FOCUS_COLOR));
	}

	if (wcscmp(config.GetStringParam(APP_LANGUAGE).c_str(), L"FR"))
		lang = FR;
	else
		lang = EN;

	/*********************DICTIONNAIRE***********************/
	{
		the_dico_filename = config.GetStringParam(DICTIONNARY_FILE_NAME);
		LoadDico(the_dico_filename);
	}

	/*********************FENETRE PRINCIPALE*****************/
	{
		mainDlg = CreateDialog(hInst
			, MAKEINTRESOURCE(IDD_MAIN_DIALOG)
			, NULL
			, (DLGPROC)DlgMainProc);
		SetWindowLongPtr(mainDlg, GWL_EXSTYLE, WS_EX_LAYERED);
		SetLayeredWindowAttributes(mainDlg, NULL, config.GetUnsignedIntParam(DIALOG_OPACITY), LWA_ALPHA);

		// Sauvegarde de la hauteur d'un bouton au demarrage.
		// Cette valeur est utilisee pour interdire une trop grande reduction
		// de la fenetre principale.
		RECT ctrlRect; GetClientRect(GetDlgItem(mainDlg,IDC_BUTTON_F1),&ctrlRect);
		heightButtonDefault = ctrlRect.bottom;

		// Mise en place des parametres de gestion d'interface
		gestionnaireBoutons.SetDialogue(mainDlg);
		gestionnaireBoutons.SetIdBase(IDC_BUTTON_F1);

		// Dimensionnement du dialogue principal
		AdapterDialoguePrincipale(false); // Initialisation de variables
		AdapterDialoguePrincipale(true); // Adaptation finale

		// Creation d'un icone d'application dans la barre des taches
		mainDlgSubMenu = GetSubMenu(LoadMenu(hInst, (LPCTSTR)IDR_MENU_MINIMIZE), 0);
		AddTaskBarIcon();	
	}



	/*********************APPLICATION************************/
	InitHook(mainDlg);
	for (int i = 0; i < 10; i++) {
		SetWindowSubclass(GetDlgItem(mainDlg, i+IDC_BUTTON_F1), TheButtonSubclass, i, (DWORD)mainDlg);
	}
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) == TRUE) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//HACCEL hAccel;
	//hAccel = LoadAccelerators(hInstance, L"DictionnaireICOM");
	//while(GetMessage(&msg,NULL,0,0) == TRUE) {
	//	if (!TranslateAccelerator(msg.hwnd, hAccel, &msg)) {
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}
	//}
	config.SetStringParam(DICTIONNARY_FILE_NAME, the_dico_filename);
	SaveDico(the_dico_filename);
	config.SaveIntoFile(CONFIG_FILE_NAME);

	clock_t tempsAvant = clock();
	HWND cacheDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_DEMARRAGE), mainDlg, (DLGPROC)DlgQuitProc);
	clock_t tempsAttente = 1000 - (clock() - tempsAvant);

	/*********************LIBERATION DES RESSOURCES**********/
	// Liberation de la DLL de hook
	EndHook();
	FreeLibrary(hinstDLLHook);

	// Liberation des ressources graphiques
	DestroyMenu(mainDlgSubMenu);

	ReleaseMutex(mutex);

	if (tempsAttente > 0) Sleep(tempsAttente); // Affichage mini de 1s
	DestroyWindow(cacheDialog);
	return (int)msg.wParam;
}

