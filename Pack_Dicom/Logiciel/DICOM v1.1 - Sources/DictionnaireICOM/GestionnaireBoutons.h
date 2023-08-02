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
/*	GestionnaireBoutons.h - Header la classe de gestionnaire de boutons	*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* ------------------------------------------------------------ INCLUDE */
/* ---------------------------------------------------- Include système */
#include <windows.h> // COLORREF etc.
#include <string>

/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/* --------------------------------------------------------- Constantes */
#define NB_MAX_BUTTONS 10 // Nombre maximum de bouttons dans l'interface

class GestionnaireBoutons
{
/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/* -------------------------------------------------------------------- */
public:
/*------------------------------------------------- Fonctions publiques */

	/**
	 * @brief Constructeur de la classe de gestion des boutons.
	 * Initialise toutes les variables membres.
	 */
	GestionnaireBoutons()
		: m_colorText(RGB(255, 255, 255))
		, m_colorBk(RGB(34, 34, 255))
		, m_nbButtons(0)
		, m_police("Comic Sans MS")
		, m_policeBold(false)
		, m_policeItalic(false)
	{}

	/**
	 * @brief Cette methode renvoie le texte present dans le bouton auquel
	 * est enleve si possible la chaine "effacer".
	 * @param i Index du bouton.
	 * @param effacer Chaine a supprimer du resultat.
	 * @return XOR de la chaine existante et la chaine a supprimer.
	 */
	std::string GetBoutonText(unsigned int i, std::string effacer);

	/**
	 * @brief Cette methode affecte un texte a un bouton donne.
	 * @param texte Texte a affecter.
	 * @param i Index du bouton.
	 */
	void SetBoutonText(unsigned int i, std::string & texte);

	/**
	 * @brief Cette methode permet de lire la police pour les bouttons
	 * @return Police a utiliser au format string.
	 */
	std::string GetPolice() {return m_police;};

	/**
	 * @brief Cette methode permet de definir la police pour les bouttons
	 * @param police Police a utiliser.
	 */
	 void SetPolice(std::string police) {m_police = police;};

	/**
	 * @brief Cette methode permet de savoir si la police est en italique
	 * @return TRUE si la police est italique, FALSE sinon.
	 */
	 bool GetPoliceItalic() {return m_policeItalic;};

	/**
	 * @brief Cette methode permet de definir si la police est en italique
	 * @param italic TRUE si italique, FALSE sinon.
	 */
	 void SetPoliceItalic(bool italic) {m_policeItalic = italic;};

	/**
	 * @brief Cette methode permet de savoir si la police est en gras
	 * @return TRUE si la police est gras, FALSE sinon.
	 */
	 bool GetPoliceBold() {return m_policeBold;};

	/**
	 * @brief Cette methode permet de definir si la police est en gras
	 * @param bold TRUE si gras, FALSE sinon.
	 */
	 void SetPoliceBold(bool bold) {m_policeBold = bold;};

	/**
	 * @brief Cette fonction a pour role de dessiner le contenu d'un bouton.
	 * (Car Microsoft est pas capable de nous fournir une API !)
	 * @param lpdis Information sur l'element de dessin.
	 * @param i Index du bouton a dessiner.
	 */
	void DessinerBouton(LPDRAWITEMSTRUCT lpdis, unsigned int i);

	/**
	 * @brief Cette methode applique la font courante a tous les boutons.
	 * @param supprimerFont Indique s'il faut supprimer les anciennes font.
	 */
	void AppliquerFont(bool supprimerFont = true);

	/**
	 * @brief Cette fonction a pour role de definir l'emplacement des boutons dans le
	 * dialogue fournit. Cette procedure redefinie la police de chaque boutton
	 * afin que cette derniere soit la plus adaptee a la taille des bouttons.
	 */
	void PositionnerBoutons();

	/**
	 * @brief Cette methode permet de definir le dialogue conteneur des boutons.
	 * @param hdlg Dialogue conteneur.
	 */
	void SetDialogue(HWND hDlg);

	/**
	 * @brief Cette methode permet de definir l'id de base pour tous les boutons.
	 * @param IDC_BUTTON_BASE ID du premier bouton.
	 */
	void SetIdBase(unsigned int IDC_BUTTON_BASE);

	/**
	 * @brief Cette methode permet de definir le nombre de boutons a afficher.
	 * @param nbBoutons Nombre de boutons a afficher.
	 */
	void SetNbBoutons(unsigned int nbBoutons) {m_nbButtons = nbBoutons;};

	/**
	 * @brief Cette methode fournie le nombre de boutons a afficher.
	 * @return Retourne le nombre de boutons a afficher.
	 */
	unsigned int GetNbBoutons() {return m_nbButtons;};

	/**
	 * @brief Cette fonction a pour role de lire la couleur du texte
	 * pour chaque boutons.
	 * @return Couleur du texte au format COLORREF.
	 */
	COLORREF GetCouleurTexte() {return m_colorText;};

	/**
	 * @brief Cette fonction a pour role de definir la couleur du texte
	 * pour chaque boutons.
	 * @param couleurTexte Couleur du texte.
	 */
	void SetCouleurTexte(COLORREF couleurTexte);

	/**
	 * @brief Cette fonction a pour role de lire la couleur du fond
	 * pour chaque boutons.
	 * @return Couleur du fond au format COLORREF.
	 */
	COLORREF GetCouleurFond() {return m_colorBk;};

	/**
	 * @brief Cette fonction a pour role de definir la couleur de fond
	 * pour chaque boutons.
	 * @param couleurFond Couleurd de fond du bouton.
	 */
	void SetCouleurFond(COLORREF couleurFond);
	
/* -------------------------------------------------------------------- */
/*                        PRIVATE                                       */
/* -------------------------------------------------------------------- */
private:
/*------------------------------------------------- Fonctions privées   */
	/**
	 * @brief Cette fonction est appelee par PositionnerBouttons elle modifie
	 * les parametres founient afin de conserver l'etat d'avancement du
	 * positionnement.
	 * @param x
	 * @param y
	 * @param cx
	 * @param cy
	 * @param dlgRect
	 * @param hDlg
	 * @param IDC_BUTTON
	 * @param maxLength
	 */
	void positionnerBouton(int & x
		, int & y
		, int & cx
		, int & cy
		, RECT & dlgRect
		, unsigned int idButton
		, int height
		, int width);

/*---------------------------------------------------- Membres privés	*/
	std::string m_textButtons[NB_MAX_BUTTONS]; // Texte a afficher pour chaque boutton
	std::string m_police; // Police a utiliser pour ecrire
	bool m_policeBold; // Police en gras
	bool m_policeItalic; // Police en italique
	COLORREF m_colorText; // Couleur du texte des bouttons
	COLORREF m_colorBk; // Couleur de fond des bouttons
	HWND m_hDlg; // Dialogue conteneur des boutons
	unsigned int m_nbButtons; // Nombre de boutons affiches
	unsigned int m_idBase; // Id de base pour tous les autres boutons
};