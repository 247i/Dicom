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
/*		Configuration.h - Header la classe de gestion de configuration	*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* ------------------------------------------------------------ INCLUDE */
/* ------------------------------------------------------- STL includes */
#include <map>
#include <string>

/* --------------------------------------------------------- CONSTANTES */
// Define des constantes de type string
#define FONT_TYPE "config.font.type"

// Define des constantes de type unsigned int
#define FONT_TEXT_COLOR "config.font.text.color"
#define DIALOG_POS_X "config.dialog.pos.x"
#define DIALOG_POS_Y "config.dialog.pos.y"
#define DIALOG_WIDTH "config.dialog.size.x"
#define DIALOG_HEIGHT "config.dialog.size.y"
#define DIALOG_BACKGROUND_COLOR "config.dialog.background.color"
#define SPLASH_NB_LETTERS "config.splash.nbletters"
#define SPLASH_TIME "config.splash.time"

// Define des constantes de type bool
#define FONT_BOLD "config.font.bold"
#define FONT_ITALIC "config.font.italic"
#define SPLASH_AUTO_INSERT "config.splash.autoinsert"

class Configuration
{
/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/* -------------------------------------------------------------------- */
public:
/*------------------------------------------------- Fonctions publiques */
	/*
	 * @brief Constructeur de la classe Configuration.
	 * @param fichierConfig Chemin d'acces au fichier de configuration.
	 */
	Configuration(std::string fichierConfig);

	/* @brief Destructeur. */
	~Configuration();

	/*
	 * @brief Sauvegarde la configuration actuelle dans fichierConfig.
	 * @param fichierConfig Fichier dans lequel sauvegarder la config.
	 * @return FALSE si la sauvegarde echoue. TRUE sinon.
	 */
	bool SaveIntoFile(std::string fichierConfig);

	/*
	 * @brief Renvoi un parametre de type string.
	 * @param paramName Nom du parametre a retourner.
	 * @return Renvoi une string representant le parametre demande.
	 */
	std::string GetStringParam(std::string paramName);

	/*
	 * @brief Met a jour une parametre de type string.
	 * @param paramName Nom du parametre a modifier.
	 * @param paramValue Valeur du paramatre.
	 */
	void SetStringParam(std::string paramName, std::string paramValue);

	/*
	 * @brief Renvoi un parametre de type unsigned int.
	 * @param paramName Nom du parametre a retourner.
	 * @return Renvoi un unsigned int representant le parametre demande.
	 */
	unsigned int GetUnsignedIntParam(std::string paramName);

	/*
	 * @brief Met a jour une parametre de type unsigned int.
	 * @param paramName Nom du parametre a modifier.
	 * @param paramValue Valeur du paramatre.
	 */
	void SetUnsignedIntParam(std::string paramName, unsigned int paramValue);

	/*
	 * @brief Renvoi un parametre de type bool.
	 * @param paramName Nom du parametre a retourner.
	 * @return Renvoi un bool representant le parametre demande.
	 */
	bool GetBoolParam(std::string paramName);

	/*
	 * @brief Met a jour une parametre de type bool.
	 * @param paramName Nom du parametre a modifier.
	 * @param paramValue Valeur du paramatre.
	 */
	void SetBoolParam(std::string paramName, bool paramValue);

/* -------------------------------------------------------------------- */
/*                        PRIVATE                                       */
/* -------------------------------------------------------------------- */
private:
/*------------------------------------------------- Fonctions privées   */
	/* @brief Charge la configuration par default. */
	void loadDefault();

/*---------------------------------------------------- Membres privés	*/
	// Nom du fichier de configuration
	std::string mFichierConfig;
	// Differentes maps contenant les parametres de configuration par types
	std::map<std::string, std::string> mMapString;
	std::map<std::string, unsigned int> mMapUnsigned;
	std::map<std::string, bool> mMapBool;
};