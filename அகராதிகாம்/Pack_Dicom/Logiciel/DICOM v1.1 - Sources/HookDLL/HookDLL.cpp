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
/*				HookDLL.cpp - Code source de la DLL de hook				*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* ------------------------------------------------------------ INCLUDE */
/* --------------------------------------------------- WINDOWS includes */
#include <windows.h> //DLL windows

/* ----------------------------------------------------- DICOM includes */
#include "../DictionnaireIcom/resource.h"

/* ----------------------------------------------VARIABLES MULTITHREADS */
#pragma data_seg("SectionVariables")
HHOOK mouseHookHandle = NULL; // Handle du hook de la souris 
HHOOK keybdHookHandle = NULL; // Handle du hook du clavier
HWND hDestWindow = NULL; // Handle de la fenêtre a qui le hook donne les données
HINSTANCE hInstFirst = NULL; // Handle d'intance de la DLL
#pragma comment(linker, "/section:SectionVariables,rws")

/*--------------------------------------------------- Fonctions privees */

/*
 * @brief Fonction est dediee a l'interception des messages venant de la souris.
 * Peut être utilisee dans les evolutions futures du logiciel.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
LRESULT CALLBACK MouseProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	return CallNextHookEx(mouseHookHandle,nCode,wParam,lParam);
}

/*
 * @brief Cette fonction est dediee a l'interception des messages venant du clavier.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
LRESULT CALLBACK KeybdProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	BYTE keyState[256]; // Etat des 256 touches du clavier
    static BOOL deadkey; // Est-ce qu'on a traité une DEAD KEY
	static bool majPressed = false; // Indique si majuscule est presse si deadkey
    WORD touche=0; // Buffer pour la traduction de la touche (ToAscii)
    char nomTouche[16]; // Buffer pour la traduction de la touche (GetKeyNameText)

    // On ne fait rien dans ce cas (cf aide API)
    if( nCode < 0 || nCode == HC_NOREMOVE )
        return CallNextHookEx( keybdHookHandle, nCode, wParam, lParam );

    // Pour éviter les répétitions
    // Bit 30 : Spécifie l'état précédent de la touche (si TRUE, on passe notre chemin)
    if( ((DWORD)lParam & 1<<30) != FALSE )
        return CallNextHookEx( keybdHookHandle, nCode, wParam, lParam );
    
    // Si c'est une DEAD KEY, on passe notre chemin
	unsigned int code = (unsigned int)MapVirtualKey ( (UINT)wParam, 2 );
    // Windows 95 retourne 0x8000, NT retourne 0x80000000
    if( (code & 0x80008000) ? TRUE : FALSE )
    {
		deadkey = TRUE;
		if (GetKeyState(VK_SHIFT)&0x8000) // Pressed
			majPressed = true;
        return CallNextHookEx( keybdHookHandle, nCode, wParam, lParam );
    }

	// Les touches detournees sont ignorees
    switch(wParam)
    {
	case VK_BACK    : // 0x08
		SendMessage(hDestWindow, PM_AFFICHERRECHERCHE, (WPARAM)DELETELASTCHAR, 0L);
		break;
	case VK_NUMPAD0 : // 0x60
    case VK_NUMPAD1 : // 0x61
    case VK_NUMPAD2 : // 0x62
    case VK_NUMPAD3 : // 0x63
    case VK_NUMPAD4 : // 0x64
    case VK_NUMPAD5 : // 0x65
    case VK_NUMPAD6 : // 0x66
    case VK_NUMPAD7 : // 0x67
    case VK_NUMPAD8 : // 0x68
    case VK_NUMPAD9 : // 0x69
	case VK_SPACE	: // 0x20
    case VK_TAB     : // 0x09
    case VK_RETURN  : // 0x0D
    case VK_PRIOR   : // 0x21
    case VK_NEXT    : // 0x22
    case VK_END     : // 0x23
    case VK_HOME    : // 0x24
    case VK_LEFT    : // 0x25 //@TODO
    case VK_RIGHT   : // 0x27 //@TODO
    case VK_DELETE  : // 0x2E //@TODO
    case VK_MULTIPLY: // 0x6A
    case VK_ADD     : // 0x6B
    case VK_SUBTRACT: // 0x6D
    case VK_DECIMAL : // 0x6E
    case VK_DIVIDE  : // 0x06
		SendMessage(hDestWindow, PM_AFFICHERRECHERCHE, (WPARAM)RESETWORD, 0L);
		break;
	case VK_SHIFT   : // 0x10
	case VK_CONTROL : // 0x11
	case VK_MENU    : // 0x12
    case VK_PAUSE   : // 0x13
    case VK_CAPITAL : // 0x14
	case VK_ESCAPE  : // 0x1B
	case VK_UP      : // 0x26
	case VK_DOWN    : // 0x28
	case VK_SNAPSHOT: // 0x2C
    case VK_INSERT  : // 0x2D
	case VK_LWIN    : // 0x5B
    case VK_RWIN    : // 0x5C
    case VK_APPS    : // 0x5D
	case VK_F12		: // 0x7B
    case VK_F13     : // 0x7C
    case VK_F14     : // 0x7D
    case VK_F15     : // 0x7E
    case VK_F16     : // 0x7F
    case VK_F17     : // 0x80
    case VK_F18     : // 0x81
    case VK_F19     : // 0x82
    case VK_F20     : // 0x83
    case VK_F21     : // 0x84
    case VK_F22     : // 0x85
    case VK_F23     : // 0x86
    case VK_F24     : // 0x87
    case VK_NUMLOCK : // 0x90
    case VK_ATTN    : // 0xF6
		break;
    case VK_F1      : // 0x70
		SendMessage(hDestWindow, WM_COMMAND, IDC_BUTTON_F1, 0L); // bouton 1
		return TRUE;
    case VK_F2      : // 0x71
		SendMessage(hDestWindow, WM_COMMAND,IDC_BUTTON_F1+1, 0L); // bouton 2
		return TRUE;
    case VK_F3      : // 0x72
		SendMessage(hDestWindow, WM_COMMAND,IDC_BUTTON_F1+2, 0L); // bouton 3
		return TRUE;
    case VK_F4      : // 0x73
		SendMessage(hDestWindow, WM_COMMAND,IDC_BUTTON_F1+3, 0L); // bouton 4
		return TRUE;
    case VK_F5      : // 0x74
		SendMessage(hDestWindow, WM_COMMAND, IDC_BUTTON_F1+4, 0L); // bouton 5
		return TRUE;
    case VK_F6      : // 0x75
		SendMessage(hDestWindow, WM_COMMAND, IDC_BUTTON_F1+5, 0L); // bouton 6
		return TRUE;
    case VK_F7      : // 0x76   
		SendMessage(hDestWindow, WM_COMMAND, IDC_BUTTON_F1+6, 0L); // bouton 7
		return TRUE;
    case VK_F8      : // 0x77
		SendMessage(hDestWindow, WM_COMMAND, IDC_BUTTON_F1+7, 0L); // bouton 8
		return TRUE;  
    case VK_F9      : // 0x78   
		SendMessage(hDestWindow, WM_COMMAND, IDC_BUTTON_F1+8, 0L); // bouton 9
		return TRUE;  
    case VK_F10     : // 0x79      
		SendMessage(hDestWindow, WM_COMMAND, IDC_BUTTON_F1+9, 0L); // bouton 10
		return TRUE;  
	case VK_F11     : // 0x7B
		SendMessage(hDestWindow, PM_AUTOINSERT, (WPARAM)INSERTWORD, 0L);
		return TRUE;
	default			:
		// On réinitialise notre tableau
        memset(keyState, 0, sizeof(keyState));
        
        if (GetKeyboardState(keyState)) {
            // ce n'est pas une DEAD KEY, on peut utiliser ToAscii
            if (!deadkey) {
                ToAscii( (UINT)wParam,(UINT)((lParam<<8)>>24),keyState,&touche,0 );
				SendMessage(hDestWindow, PM_AFFICHERRECHERCHE, (WPARAM)APPENDCHAR
					, (LPARAM)touche);
            }
			else { // sinon, on doit utiliser autre chose !
				memset( nomTouche, 0, sizeof(nomTouche) );
                GetKeyNameText( (LONG)lParam, nomTouche, 256 );

				// Recupere l'etat de shift
				bool majLettrePressed = false;
				if (GetKeyState(VK_SHIFT)&0x8000) // Pressed
					majLettrePressed = true;

				// Gestion des cas speciaux
				if (false == majPressed) { // apostrophe
					switch(nomTouche[0]) {
						case 'A': majLettrePressed?nomTouche[0] = 'Â':nomTouche[0] = 'â'; break;
						case 'E': majLettrePressed?nomTouche[0] = 'Ê':nomTouche[0] = 'ê'; break;
						case 'I': majLettrePressed?nomTouche[0] = 'Î':nomTouche[0] = 'î'; break;
						case 'O': majLettrePressed?nomTouche[0] = 'Ô':nomTouche[0] = 'ô'; break;
						case 'U': majLettrePressed?nomTouche[0] = 'Û':nomTouche[0] = 'û'; break;
						// Pas pour y					
					}
				}
				else { // Deux points
					switch(nomTouche[0]) {
						case 'A': majLettrePressed?nomTouche[0] = 'Ä':nomTouche[0] = 'ä'; break;
						case 'E': majLettrePressed?nomTouche[0] = 'Ë':nomTouche[0] = 'ë'; break;
						case 'I': majLettrePressed?nomTouche[0] = 'Ï':nomTouche[0] = 'ï'; break;
						case 'O': majLettrePressed?nomTouche[0] = 'Ö':nomTouche[0] = 'ö'; break;
						case 'U': majLettrePressed?nomTouche[0] = 'Ü':nomTouche[0] = 'ü'; break;
						case 'Y': majLettrePressed?nomTouche[0] = 'Y':nomTouche[0] = 'ÿ'; break;
					}
				}

				// Notifie DICOM
				SendMessage(hDestWindow, PM_AFFICHERRECHERCHE, (WPARAM)APPENDCHAR
					, (LPARAM)(*((unsigned int*)nomTouche)));
				deadkey = FALSE;
				majPressed = false;
            }
        }
		break;
    }

    return CallNextHookEx( keybdHookHandle, nCode, wParam, lParam );
}


/*------------------------------------------------- Fonctions publiques */

/*
 * @brief Cette fonction est accessible a l'exterieur de la DLL. Elle initialise le 
 * deroutement des messages venant du clavier et de la souris vers le HWND fournit.
 * @param hDest Fenetre sur laquelle renvoyer les messages recus.
 *
 */
_declspec(dllexport) void InitHook(HWND hDest)
{
	// Installation du hook sur la souris
	if (NULL == mouseHookHandle){
		mouseHookHandle = SetWindowsHookEx(WH_MOUSE // Hook sur la souris
			, (HOOKPROC)MouseProc // Utiliser la fonction MouseProc
			, hInstFirst // Dans la DLL d'instance HInst
			, 0); // Pour tous les threads
	}

    // Installation du hook pour le clavier
	if (NULL == keybdHookHandle) {
		keybdHookHandle = SetWindowsHookEx(WH_KEYBOARD // Hook sur le clavier
			, (HOOKPROC)KeybdProc // Utiliser la fonction KeybdProc
			, hInstFirst // Dans la DLL d'instance HInst
			, 0); // Pour tous les threads
	}

    // Sauvegarde de la fenetre destinatrice des messages
    hDestWindow = hDest;
}

/*
 * @brief Cette fonction est accessible a l'exterieur de la DLL. Elle met fin au 
 * deroutement des messages venant du clavier et de la souris.
 */
_declspec(dllexport) void EndHook()
{
    // Supression des hooks
	if( NULL!=mouseHookHandle )
		UnhookWindowsHookEx(mouseHookHandle);
	if( NULL!=keybdHookHandle )
		UnhookWindowsHookEx(keybdHookHandle);

	// RAZ
	mouseHookHandle = NULL;
	keybdHookHandle = NULL;
}

/*
 * @brief Point d'entree de la DLL de hook DICOM.
 * @param all Voir MSDN.
 * @return Voir MSDN.
 */
int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, PVOID pReserved)
{
	if(DLL_PROCESS_ATTACH == dwReason) { // à la création de l'instance de la DLL
		hInstFirst = hInstance;
	}

	if(DLL_PROCESS_DETACH == dwReason) { // a la destruction de l'instance de la DLL
	}
	return 1;
}