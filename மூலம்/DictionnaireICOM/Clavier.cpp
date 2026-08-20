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
/*			Clavier.cpp - Classe de simulation du clavier				*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* -------------------------------------------------- Include personnel */
#include "Clavier.h"


/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/*------------------------------------------------- Fonctions publiques */
void Clavier::SimulerTexte(const wchar_t * texte)
{
	Clavier::kbdSimulerTexte(texte);
}

void Clavier::SimulerBackSpace() {
	/*INPUT input;
	input.ki.dwExtraInfo = 0;
	input.ki.time = 0;
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 0;

	input.ki.wScan = VK_BACK;

	input.ki.dwFlags = KEYEVENTF_UNICODE;
	SendInput(1, &input, sizeof(INPUT));
	input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));*/
	keybd_event(VK_BACK, 0, WM_KEYDOWN, 0);
	Sleep(20);
	keybd_event(VK_BACK, 0, KEYEVENTF_KEYUP, 0);
}

void Clavier::SimulerTouche(wchar_t touche)
{
	keybd_event((BYTE)touche, 0, WM_KEYDOWN, 0); // on simule la touche
	Sleep(20);
	keybd_event((BYTE)touche, 0, KEYEVENTF_KEYUP, 0); // on relache la touche
}

/* -------------------------------------------------------------------- */
/*                        PRIVE                                         */
/* --------------------------------------------------------- Constantes */
/* -------------------------------------------------------------- Types */
/*--------------------------------------------------- Fonctions privees */


void Clavier::kbdSimulerTexte(const wchar_t * texte)
{

	// Definition du type d'evenement
	INPUT inputEvent;
	inputEvent.ki.dwExtraInfo = 0;
	inputEvent.ki.time = 0;
	inputEvent.type = INPUT_KEYBOARD;
	inputEvent.ki.wVk = 0;
	
	// Tant qu'il y a des caractere a traiteer
  while(*texte) {
//#ifdef _UNICODE
	inputEvent.ki.wScan = (WORD) (BYTE) *texte;
//#else
  //  inputEvent.ki.wScan = (WORD) (BYTE) *texte;
//#endif
    inputEvent.ki.dwFlags = KEYEVENTF_UNICODE;
    
	// Emission sur le clavier
	SendInput(1, &inputEvent, sizeof(INPUT));
    inputEvent.ki.dwFlags =  KEYEVENTF_UNICODE| KEYEVENTF_KEYUP;
    SendInput(1, &inputEvent, sizeof(INPUT));

	// Prepare la lecture du prochain caractere
	texte++;
  }
}
