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
#include <windows.h>

using namespace std;

/* --------------------------------------------------------- CONSTANTES */
// Define des constantes de type string
#define FONT_TYPE						L"config.font.type"
#define DICTIONNARY_FILE_NAME			L"config.dictionnary.file.name"
#define VOICE_SELECTED_VOICE			L"config.voice.selected"
#define APP_LANGUAGE					L"config.app.lang"

// Define des constantes de type unsigned int
#define FONT_TEXT_COLOR					L"config.font.text.color"
#define DIALOG_POS_X					L"config.dialog.pos.x"
#define DIALOG_POS_Y					L"config.dialog.pos.y"
#define DIALOG_WIDTH					L"config.dialog.size.x"
#define DIALOG_HEIGHT					L"config.dialog.size.y"
#define DIALOG_BACKGROUND_COLOR			L"config.dialog.background.color"
#define DIALOG_BACKGROUND_FOCUS_COLOR	L"config.dialog.background.focus.color"
#define FONT_TEXT_FOCUS_COLOR			L"config.font.text.focus.color"
#define DIALOG_OPACITY					L"config.dialog.opacity"
#define SPLASH_NB_LETTERS				L"config.splash.nbletters"
#define SPLASH_TIME						L"config.splash.time"
#define NB_WORD							L"config.boutons.max"
#define VOICE_VOLUME					L"config.voice.volume"
#define VOICE_RATE						L"config.voice.rate"


// Define des constantes de type bool
#define FONT_BOLD						L"config.font.bold"
#define FONT_ITALIC						L"config.font.italic"
#define SPLASH_AUTO_INSERT				L"config.splash.autoinsert"
#define VOICE_ACTIVATION_HOVER			L"config.voice.activation.hover"
#define VOICE_ACTIVATION_INSERT			L"config.voice.activation.insert"
#define VOICE_RATE_NEGATIVE				L"config.voice.rate.negative"
#define SELECT_BY_FUNCTION				L"config.select.function"
#define SELECT_BY_CLIC					L"config.select.clic"
#define SELECT_BY_ARROWS				L"config.select.arrows"

class Configuration
{
/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/* -------------------------------------------------------------------- */
public:
/*------------------------------------------------- Fonctions publiques */
	
	Configuration();
	/*
	 * @brief Constructeur de la classe Configuration.
	 * @param fichierConfig Chemin d'acces au fichier de configuration.
	 */
	Configuration(const wchar_t * fichierConfig);

	/* @brief Destructeur. */
	~Configuration();

	/*
	 * @brief Sauvegarde la configuration actuelle dans fichierConfig.
	 * @param fichierConfig Fichier dans lequel sauvegarder la config.
	 * @return FALSE si la sauvegarde echoue. TRUE sinon.
	 */
	bool SaveIntoFile(const wchar_t *fichierConfig);

	/*
	 * @brief Renvoi un parametre de type string.
	 * @param paramName Nom du parametre a retourner.
	 * @return Renvoi une string representant le parametre demande.
	 */
	wstring GetStringParam(wstring paramName);

	/*
	 * @brief Met a jour une parametre de type string.
	 * @param paramName Nom du parametre a modifier.
	 * @param paramValue Valeur du paramatre.
	 */
	void SetStringParam(wstring paramName, wstring paramValue);

	/*
	 * @brief Renvoi un parametre de type unsigned int.
	 * @param paramName Nom du parametre a retourner.
	 * @return Renvoi un unsigned int representant le parametre demande.
	 */
	unsigned int GetUnsignedIntParam(wstring paramName);

	/*
	 * @brief Met a jour une parametre de type unsigned int.
	 * @param paramName Nom du parametre a modifier.
	 * @param paramValue Valeur du paramatre.
	 */
	void SetUnsignedIntParam(wstring paramName, unsigned int paramValue);

	/*
	 * @brief Renvoi un parametre de type bool.
	 * @param paramName Nom du parametre a retourner.
	 * @return Renvoi un bool representant le parametre demande.
	 */
	bool GetBoolParam(wstring paramName);

	/*
	 * @brief Met a jour une parametre de type bool.
	 * @param paramName Nom du parametre a modifier.
	 * @param paramValue Valeur du paramatre.
	 */
	void SetBoolParam(wstring paramName, bool paramValue);

	LONG GetRate();

	void SetRate(LONG rate);

/* -------------------------------------------------------------------- */
/*                        PRIVATE                                       */
/* -------------------------------------------------------------------- */
private:
/*------------------------------------------------- Fonctions privées   */
	/* @brief Charge la configuration par default. */
	void loadDefault();

/*---------------------------------------------------- Membres privés	*/
	// Nom du fichier de configuration
	const wchar_t *mFichierConfig;
	// Differentes maps contenant les parametres de configuration par types
	map<wstring, wstring> mMapString;
	map<wstring, unsigned int> mMapUnsigned;
	map<wstring, bool> mMapBool;
};