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
/*	GestionnaireBoutons.cpp - Classe de gestion des boutons dans l'IHM	*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* ------------------------------------------------------------ INCLUDE */
/* ---------------------------------------------------- Include système */
//#include <windows.h> // API windows

/* -------------------------------------------------- Include personnel */
#include "GestionnaireBoutons.h"

/**********************************Constantes bouttons************************/
/** @brief Coeficient de perte entre hauteur et largeur */
static const float coefHeightWidthFontMaj = 0.25f;
static const float coefHeightWidthFontMin = 0.15f;

/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/* -------------------------------------------------------------------- */
/*------------------------------------------------- Fonctions publiques */

wstring GestionnaireBoutons::GetBoutonText(unsigned int i, wstring effacer)
{
	// Chaine a retourner
	wstring xorReturn;

	// Taille de "F1: " par exemple
	unsigned int useless_len = 4;

	if (i >= 9) { // Plus de deux chiffres
		useless_len = 5;
	}

	xorReturn = m_textButtons[i].substr(useless_len + effacer.length()
		, m_textButtons[i].size() - (useless_len + effacer.length()));

	return xorReturn;
}

void GestionnaireBoutons::SetBoutonText(unsigned int i, wstring texte)
{
	if (i >= m_nbWord) // Ignore si non valide
		return;

	// Creation du nom dans l'ihm
	wchar_t bufferConv[12];
	m_textButtons[i].clear();
	m_textButtons[i].append(L"F");
	_itow(i + 1, bufferConv, 10);
	m_textButtons[i].append(bufferConv);
	m_textButtons[i].append(L": ");
	m_textButtons[i].append(texte);
}

void GestionnaireBoutons::DessinerBouton(LPDRAWITEMSTRUCT lpdis, unsigned int i)
{
	if (i >= m_nbWord) // Ignore si non valide
		return;

	// Recupere le texte pour ce boutton
	std::wstring texte = m_textButtons[i];

	// Déterminer les dimensions du texte:
	SIZE dims;
    GetTextExtentPoint32(lpdis->hDC, texte.c_str(), (int)texte.length(), &dims);

	// Définir la couleur du texte:
	if (i == da_button)
		SetTextColor(lpdis->hDC, m_colorFocusText);
	else
		SetTextColor(lpdis->hDC, m_colorText);

	// Définir la couleur du fond:
	if (i == da_button)
		SetBkColor(lpdis->hDC, m_colorFocusBk);
	else
		SetBkColor(lpdis->hDC, m_colorBk);
	
	// Déterminer l'état du bouton:
	BOOL etat = lpdis->itemState & ODS_SELECTED;

	// Déterminer la largeur et la hauteur du bouton:
	int largeur = lpdis->rcItem.right-lpdis->rcItem.left;
	int hauteur = lpdis->rcItem.bottom-lpdis->rcItem.top;

	// Ecrire le texte sur le bouton:
	ExtTextOut(lpdis->hDC
		, 10+etat
		, (hauteur-dims.cy)/2+etat
		, ETO_OPAQUE | ETO_CLIPPED
		, &lpdis->rcItem
		, texte.c_str()
		, (int)texte.length()
		, NULL);

    // Dessiner le cadre du bouton selon son état:
	DrawEdge(lpdis->hDC, &lpdis->rcItem,(etat ? EDGE_SUNKEN : EDGE_RAISED ), BF_RECT);
}

void GestionnaireBoutons::AppliquerFont(bool supprimerFont)
{
	// Calcul de la taille maximale de tous les bouttons
	unsigned int maxIndex = 0;
	for (unsigned int i = 0; i<m_nbButtons; ++i) {
		if(m_textButtons[i].length() > m_textButtons[maxIndex].length())
			maxIndex = i;
	}

	for (unsigned int i = 0; i<m_nbButtons; ++i) {
		// Mise en place de la police pour le bouton
		// Control sur lequel on travail
		HWND currentHwnd;
		// Taille du control
		RECT ctrlRect;
		// Recuperation du control a modifier
		currentHwnd = GetDlgItem(m_hDlg, i+m_idBase);
		// Recuperation de la taille du controle
		GetClientRect(currentHwnd,&ctrlRect);

		// Taille de caractere
		int height = 0;
		int width = 0;
		float currentCoef = coefHeightWidthFontMin;

		// Index de recherche de majuscule
		unsigned int majBase = 0;
		if (maxIndex >= 9)
			majBase = 5;
		else
			majBase = 4;
			
		// Verifie si le texte est en majuscule
		if (m_textButtons[maxIndex].length() >= (majBase + 2)) {
			std::locale loc("French_France");
			if (isupper(m_textButtons[maxIndex][majBase],loc)) {
				if(isupper(m_textButtons[maxIndex][majBase + 1],loc)) {
					currentCoef = coefHeightWidthFontMaj; // Applique un coef maj
				}		
			}
		}

		// Calcul des dimensions des polices
		height = (int)(ctrlRect.bottom-(currentCoef*ctrlRect.bottom));
		width = (int)((ctrlRect.right-10-(currentCoef*ctrlRect.right))/(m_textButtons[maxIndex].length()));
		int fontMaxWidth = (int)((height-(currentCoef*height)));
		if (width > fontMaxWidth)
			width = fontMaxWidth;

		// Creation d'une police d'ecriture adaptee
		HFONT hFont = CreateFont(height
			, width
			, 0
			, 0
			, m_policeBold?FW_BOLD:FW_NORMAL
			, m_policeItalic
			, FALSE
			, FALSE
			, DEFAULT_CHARSET
			, OUT_DEFAULT_PRECIS
			, CLIP_DEFAULT_PRECIS
			, ANTIALIASED_QUALITY //DEFAULT_QUALITY
			, DEFAULT_PITCH
			, m_police.c_str()); //NULL);

		// Recuperation et destruction de l'ancienne police
		HFONT hFontOld = (HFONT)SendMessage(currentHwnd, WM_GETFONT, 0, 0);		

		// Mise en place de la nouvelle police
		SendMessage(currentHwnd, WM_SETFONT, (WPARAM)hFont, 1);

		if (supprimerFont)
			DeleteObject(hFontOld); // Suppression de l'ancienne font apres dessin
	}
}

void GestionnaireBoutons::SetBoutonFocus(unsigned int i) {
	da_button = i;
}

void GestionnaireBoutons::PositionnerBoutons()
{
	// Variables pour les deplacements
	int x; // Position d'insertion du controle sur x
	int y; // Position d'insertion sur y
	int cx; // Largeur du controle
	int cy; // Hauteur du controle

	// Recuperation de la taille du dialogue courant
	RECT dlgRect;
	GetClientRect(m_hDlg,&dlgRect);

	// Calcul de la taille maximale de tous les bouttons
	unsigned int maxIndex = 0;
	for (unsigned int i=0; i<m_nbButtons; ++i) {
		if(m_textButtons[i].length() > m_textButtons[maxIndex].length())
			maxIndex = i;
	}

	// Taille de caractere
	int height = 0;
	int width = 0;
	float currentCoef = coefHeightWidthFontMin;

	// Index de recherche de majuscule
	unsigned int majBase = 0;
	if (maxIndex >= 9)
		majBase = 5;
	else
		majBase = 4;
		
	// Verifie si le texte est en majuscule
	if (m_textButtons[maxIndex].length() >= (majBase + 2)) {
		std::locale loc("French_France");
		if (isupper(m_textButtons[maxIndex][majBase], loc)) {
			if (isupper(m_textButtons[maxIndex][majBase + 1], loc)) {
				currentCoef = coefHeightWidthFontMaj; // Applique un coef maj
			}
		}
	}

	// Calcul des dimensions des polices
	RECT ctrlRect;
	GetClientRect(GetDlgItem(m_hDlg, m_idBase), &ctrlRect);
	height = (int)(ctrlRect.bottom - (currentCoef*ctrlRect.bottom));
	width = (int)((ctrlRect.right - 10 - (currentCoef*ctrlRect.right)) / (m_textButtons[maxIndex].length()));
	int fontMaxWidth = (int)((height - (currentCoef*height)));
	if (width > fontMaxWidth)
		width = fontMaxWidth;

	// Mise en place des differents bouttons
	for (unsigned int i=0; i<m_nbButtons; ++i) {
		positionnerBouton(x, y, cx, cy, dlgRect, i, height, width);
	}
}

void GestionnaireBoutons::SetDialogue(HWND hDlg)
{
	m_hDlg = hDlg;
}

void GestionnaireBoutons::SetIdBase(unsigned int IDC_BUTTON_BASE)
{
	m_idBase = IDC_BUTTON_BASE;
}

void GestionnaireBoutons::SetCouleurText(COLORREF couleurText)
{
	m_colorText = couleurText;
}

void GestionnaireBoutons::SetCouleurBk(COLORREF couleurBk)
{
	m_colorBk = couleurBk;
}

void GestionnaireBoutons::SetCouleurFocusBk(COLORREF couleurFocusBk) {
	m_colorFocusBk = couleurFocusBk;
}

void GestionnaireBoutons::SetCouleurFocusText(COLORREF couleurFocusText) {
	m_colorFocusText = couleurFocusText;

}

unsigned int GestionnaireBoutons::GetBoutonFocus() { 
	return da_button; 
}

void GestionnaireBoutons::SetNbBoutons(unsigned int nbBoutons) {
	m_nbButtons = nbBoutons <= m_nbWord ? nbBoutons : m_nbWord;
}

void GestionnaireBoutons::SetNbWord(unsigned int max) {
	m_nbWord = max;
}

/* -------------------------------------------------------------------- */
/*                        PRIVATE                                       */
/* -------------------------------------------------------------------- */
/*------------------------------------------------- Fonctions privées   */
void GestionnaireBoutons::positionnerBouton(int & x
	, int & y
	, int & cx
	, int & cy
	, RECT & dlgRect
	, unsigned int idButton
	, int height
	, int width)
{
	// Control sur lequel on travail
	HWND currentHwnd;
	// Recuperation du control a modifier
	currentHwnd = GetDlgItem(m_hDlg, idButton+m_idBase);

	if(idButton == 0) { // Premier boutton
		// Calcul de la nouvelle position
		x = 10;
		y = 10;
		cx = dlgRect.right-20;
		cy = (dlgRect.bottom-20)/(m_nbButtons==0?1:m_nbButtons);
	}
	else { // Bouttons suivant
		//Calcul de la nouvelle position
		y = y+cy;
	}

	// Dimensionnement du control
	MoveWindow(currentHwnd,x,y,cx,cy,TRUE);

	// Creation d'une police d'ecriture adaptee
	HFONT hFont = CreateFont(height
		, width
		, 0
		, 0
		, m_policeBold?FW_BOLD:FW_NORMAL
		, m_policeItalic
		, FALSE
		, FALSE
		, DEFAULT_CHARSET
		, OUT_DEFAULT_PRECIS
		, CLIP_DEFAULT_PRECIS
		, ANTIALIASED_QUALITY //DEFAULT_QUALITY
		, DEFAULT_PITCH
		, m_police.c_str()); //NULL);

	// Recuperation et destruction de l'ancienne police
	HFONT hFontOld = (HFONT)SendMessage(currentHwnd, WM_GETFONT, 0, 0);
	DeleteObject(hFontOld); // Suppression

	// Mise en place de la nouvelle police
	SendMessage(currentHwnd, WM_SETFONT, (WPARAM)hFont, 1);
}
