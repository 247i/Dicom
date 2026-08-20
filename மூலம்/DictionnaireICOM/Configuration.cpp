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
/*			Configuration.cpp - Classe de gestion de configuration		*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* ------------------------------------------------------------ INCLUDE */
/* ------------------------------------------------------- STL includes */
#include <fstream>

/* -------------------------------------------------- Include personnel */
#include "Configuration.h"

/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/*------------------------------------------------- Fonctions publiques */

Configuration::Configuration() {}

Configuration::Configuration(const wchar_t * fichierConfig)
{
	// Initialisation des variables membres
	mFichierConfig = fichierConfig;

	// Lecture de la configuration
	std::wifstream infile(mFichierConfig);
	if (!infile.good()) // Aucun fichier, configuration par defaut
		loadDefault();
	else { // Fichier de configuration existant
		while (!infile.eof() && infile.good()) {
			// Lecture lignes par lignes
			wchar_t line[256];
			infile.getline(line, sizeof(line));

			// Lecture de la valeur dans cette ligne
			wchar_t *value = wcschr(line, '=');
			if (NULL != value) { // Valeur trouve
				if (' ' == value[1]) value += 2; // Supprime si espace
				else value += 1;
			}
			else { // Ignore cette ligne
				continue;
			}

			// Traitement des parametres de type string
			if (wcsstr(line, FONT_TYPE) != NULL) 
				mMapString[FONT_TYPE] = value;
			else if (wcsstr(line, DICTIONNARY_FILE_NAME) != NULL)
				mMapString[DICTIONNARY_FILE_NAME] = value;
			else if (wcsstr(line, VOICE_SELECTED_VOICE) != NULL) 
				mMapString[VOICE_SELECTED_VOICE] = value;
			else if (wcsstr(line, APP_LANGUAGE) != NULL)
				mMapString[APP_LANGUAGE] = value;

			// Traitement des parametres de type unsigned int
			if (wcsstr(line, FONT_TEXT_COLOR) != NULL)
				mMapUnsigned[FONT_TEXT_COLOR] = _wtol(value);
			else if (wcsstr(line, DIALOG_POS_X) != NULL)
				mMapUnsigned[DIALOG_POS_X] = _wtol(value);
			else if (wcsstr(line, DIALOG_POS_Y) != NULL)
				mMapUnsigned[DIALOG_POS_Y] = _wtol(value);
			else if (wcsstr(line, DIALOG_WIDTH) != NULL)
				mMapUnsigned[DIALOG_WIDTH] = _wtol(value);
			else if (wcsstr(line, DIALOG_HEIGHT) != NULL)
				mMapUnsigned[DIALOG_HEIGHT] = _wtol(value);
			else if (wcsstr(line, DIALOG_BACKGROUND_COLOR) != NULL)
				mMapUnsigned[DIALOG_BACKGROUND_COLOR] = _wtol(value);
			else if (wcsstr(line, DIALOG_BACKGROUND_FOCUS_COLOR) != NULL)
				mMapUnsigned[DIALOG_BACKGROUND_FOCUS_COLOR] = _wtol(value);
			else if (wcsstr(line, FONT_TEXT_FOCUS_COLOR) != NULL)
				mMapUnsigned[FONT_TEXT_FOCUS_COLOR] = _wtol(value);
			else if (wcsstr(line, SPLASH_NB_LETTERS) != NULL)
				mMapUnsigned[SPLASH_NB_LETTERS] = _wtol(value);
			else if (wcsstr(line, SPLASH_TIME) != NULL)
				mMapUnsigned[SPLASH_TIME] = _wtol(value);
			else if (wcsstr(line, NB_WORD) != NULL)
				mMapUnsigned[NB_WORD] = _wtol(value);
			else if (wcsstr(line, DIALOG_OPACITY) != NULL)
				mMapUnsigned[DIALOG_OPACITY] = _wtol(value);
			else if (wcsstr(line, VOICE_VOLUME) != NULL)
				mMapUnsigned[VOICE_VOLUME] = _wtol(value);
			else if (wcsstr(line, VOICE_RATE) != NULL)
				mMapUnsigned[VOICE_RATE] = _wtol(value);

			// Traitement des parametres de type bool
			if (wcsstr(line, FONT_BOLD) != NULL)
				mMapBool[FONT_BOLD] = value[0] != '0';
			else if (wcsstr(line, FONT_ITALIC) != NULL)
				mMapBool[FONT_ITALIC] = value[0] != '0';
			else if (wcsstr(line, SPLASH_AUTO_INSERT) != NULL)
				mMapBool[SPLASH_AUTO_INSERT] = value[0] != '0';
			else if (wcsstr(line, VOICE_ACTIVATION_HOVER) != NULL)
				mMapBool[VOICE_ACTIVATION_HOVER] = value[0] != '0';
			else if (wcsstr(line, VOICE_ACTIVATION_INSERT) != NULL)
				mMapBool[VOICE_ACTIVATION_INSERT] = value[0] != '0';
			else if (wcsstr(line, VOICE_RATE_NEGATIVE) != NULL)
				mMapBool[VOICE_RATE_NEGATIVE] = value[0] != '0';
			else if (wcsstr(line, SELECT_BY_FUNCTION) != NULL)
				mMapBool[SELECT_BY_FUNCTION] = value[0] != '0';
			else if (wcsstr(line, SELECT_BY_CLIC) != NULL)
				mMapBool[SELECT_BY_CLIC] = value[0] != '0';
			else if (wcsstr(line, SELECT_BY_ARROWS) != NULL)
				mMapBool[SELECT_BY_ARROWS] = value[0] != '0';
			
		}
		infile.close();
	}
}

Configuration::~Configuration()
{
}

bool Configuration::SaveIntoFile(const wchar_t * fichierConfig)
{
	// Ouverture avec ecrasement du fichier de config
	std::wofstream outfile(fichierConfig);
	if (!outfile.good()) return false; // Impossible de sauvegarder

	// Sauvegarde des parametres de type string
	outfile << "[string]" << std::endl;
	for (std::map<wstring, wstring>::const_iterator iter = mMapString.begin();
		iter != mMapString.end();
		++iter) {
		outfile << iter->first << " = " << iter->second << std::endl;
	}

	// Sauvegarde des parametres de type unsigned int
	outfile << std::endl << "[unsigned]" << std::endl;
	for (std::map<wstring, unsigned int>::const_iterator iter = mMapUnsigned.begin();
		iter != mMapUnsigned.end();
		++iter) {
		outfile << iter->first << " = " << iter->second << std::endl;
	}

	// Sauvegardes des parametres de type bool
	outfile << std::endl << "[bool]" << std::endl;
	for (std::map<wstring, bool>::const_iterator iter = mMapBool.begin();
		iter != mMapBool.end();
		++iter) {
		outfile << iter->first << " = " << iter->second << std::endl;
	}

	// Sauvegarde terminee
	outfile.close();

	return true;
}

wstring Configuration::GetStringParam(wstring paramName)
{
	wstring retour;

	// Recherche du pamametre
	std::map<wstring, wstring>::const_iterator iter;
	if ((iter = mMapString.find(paramName)) != mMapString.end()) // Existe
		retour = iter->second;

	return retour;
}

void Configuration::SetStringParam(wstring paramName, wstring paramValue)
{
	mMapString[paramName] = paramValue;
}

unsigned int Configuration::GetUnsignedIntParam(wstring paramName)
{
	unsigned int retour = 0;

	// Recherche du pamametre
	std::map<wstring, unsigned int>::const_iterator iter;
	if ((iter = mMapUnsigned.find(paramName)) != mMapUnsigned.end()) // Existe
		retour = iter->second;

	return retour;
}

void Configuration::SetUnsignedIntParam(wstring paramName, unsigned int paramValue)
{
	mMapUnsigned[paramName] = paramValue;
}

bool Configuration::GetBoolParam(wstring paramName)
{
	bool retour = false;

	// Recherche du pamametre
	map<wstring, bool>::const_iterator iter;
	if ((iter = mMapBool.find(paramName)) != mMapBool.end()) // Existe
		retour = iter->second;

	return retour;
}

void Configuration::SetBoolParam(wstring paramName, bool paramValue)
{
	mMapBool[paramName] = paramValue;
}

void Configuration::SetRate(LONG rate) {
	if (rate < 0)
		SetBoolParam(VOICE_RATE_NEGATIVE, true);
	else
		SetBoolParam(VOICE_RATE_NEGATIVE, false);
	SetUnsignedIntParam(VOICE_RATE, abs(rate));
}

LONG Configuration::GetRate() {
	LONG rate = GetUnsignedIntParam(VOICE_RATE);
	if (GetBoolParam(VOICE_RATE_NEGATIVE))
		rate *= -1;
	return rate;
}

/* -------------------------------------------------------------------- */
/*                        PRIVATE                                       */
/* -------------------------------------------------------------------- */
/*------------------------------------------------- Fonctions privées   */
void Configuration::loadDefault()
{
	// Parametres string par defaut
	SetStringParam(FONT_TYPE, L"Comic Sans MS"); // Not defined
	SetStringParam(DICTIONNARY_FILE_NAME, L"DICOM_DEFAULT.dic");
	SetStringParam(VOICE_SELECTED_VOICE, L"");

	// Parametres unsigned par defaut
	SetUnsignedIntParam(FONT_TEXT_COLOR, 0); // Not defined
	SetUnsignedIntParam(DIALOG_POS_X, 0); // Not defined
	SetUnsignedIntParam(DIALOG_POS_Y, 0); // Not defined
	SetUnsignedIntParam(DIALOG_WIDTH, 0); // Not defined
	SetUnsignedIntParam(DIALOG_HEIGHT, 0); // Not defined
	SetUnsignedIntParam(DIALOG_BACKGROUND_COLOR, 16777215); // Not defined
	SetUnsignedIntParam(DIALOG_BACKGROUND_FOCUS_COLOR, 0);
	SetUnsignedIntParam(FONT_TEXT_FOCUS_COLOR, 16777215);
	SetUnsignedIntParam(SPLASH_NB_LETTERS, 0); // Affichage instantane
	SetUnsignedIntParam(SPLASH_TIME, 20000); // 20s
	SetUnsignedIntParam(NB_WORD, 10);
	SetUnsignedIntParam(DIALOG_OPACITY, 255);
	SetUnsignedIntParam(VOICE_VOLUME, 100);
	SetUnsignedIntParam(VOICE_RATE, 0);

	// Parametres bool par defaut
	SetBoolParam(FONT_BOLD, false); // Pas de police en gras
	SetBoolParam(FONT_ITALIC, false); // Pas de police en italique
	SetBoolParam(SPLASH_AUTO_INSERT, false); // Pas d'auto insert
	SetBoolParam(VOICE_ACTIVATION_HOVER, false); // Pas de synthèse vocale
	SetBoolParam(VOICE_ACTIVATION_INSERT, false); // Pas de synthèse vocale
	SetBoolParam(VOICE_RATE_NEGATIVE, false);
	SetBoolParam(SELECT_BY_FUNCTION, true);
	SetBoolParam(SELECT_BY_CLIC, true);
	SetBoolParam(SELECT_BY_ARROWS, true);
}