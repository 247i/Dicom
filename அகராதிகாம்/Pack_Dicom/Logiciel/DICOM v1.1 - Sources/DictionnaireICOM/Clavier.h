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
/*			Clavier.h - Header la classe de simulation du clavier		*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

class Clavier
{
/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/* -------------------------------------------------------------------- */
public:
/*------------------------------------------------- Fonctions publiques */
	/**
	 * @brief Cette methode a pour role la simulation de toutes les lettre
	 * presentes dans la chaine fournie sur le clavier.
	 * @param texte Texte a simuler au clavier.
	 */
	static void SimulerTexte(const char * texte);
	
/* -------------------------------------------------------------------- */
/*                        PRIVATE                                       */
/* -------------------------------------------------------------------- */
private:
/*------------------------------------------------- Fonctions privées   */
	/**
	 * @brief Cette methode a pour role de faire l'appui sur une touche 
	 * virtuelle. Methode pas tres performante.
	 * param touche touche virtuelle a simuler.
	 */
	static void kbdSimulerTouche(unsigned char touche);

	/**
	 * @brief Simuler un texte entree au clavier.
	 * @param texte Texte a simuler.
	 */
	static void kbdSimulerTexte(TCHAR *texte);
};