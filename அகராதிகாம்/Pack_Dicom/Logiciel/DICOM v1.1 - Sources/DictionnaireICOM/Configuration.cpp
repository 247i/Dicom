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
Configuration::Configuration(std::string fichierConfig)
{
	// Initialisation des variables membres
	mFichierConfig = fichierConfig;

	// Lecture de la configuration
	std::ifstream infile(mFichierConfig.c_str());
	if (!infile.good()) // Aucun fichier, configuration par defaut
		loadDefault();
	else { // Fichier de configuration existant
		while (!infile.eof() && infile.good()) {
			// Lecture lignes par lignes
			std::string line;
			std::getline(infile, line);

			// Lecture de la valeur dans cette ligne
			char * value = strchr(line.c_str(), '=');
			if (NULL != value) { // Valeur trouve
				if (' ' == value[1]) value += 2; // Supprime si espace
				else value += 1;
			}
			else { // Ignore cette ligne
				continue;
			}

			// Traitement des parametres de type string
			if (line.find(FONT_TYPE) != std::string::npos)
				mMapString[FONT_TYPE] = value;

			// Traitement des parametres de type unsigned int
			if (line.find(FONT_TEXT_COLOR) != std::string::npos)
				mMapUnsigned[FONT_TEXT_COLOR] = atol(value);
			else if (line.find(DIALOG_POS_X) != std::string::npos)
				mMapUnsigned[DIALOG_POS_X] = atol(value);
			else if (line.find(DIALOG_POS_Y) != std::string::npos)
				mMapUnsigned[DIALOG_POS_Y] = atol(value);
			else if (line.find(DIALOG_WIDTH) != std::string::npos)
				mMapUnsigned[DIALOG_WIDTH] = atol(value);
			else if (line.find(DIALOG_HEIGHT) != std::string::npos)
				mMapUnsigned[DIALOG_HEIGHT] = atol(value);
			else if (line.find(DIALOG_BACKGROUND_COLOR) != std::string::npos)
				mMapUnsigned[DIALOG_BACKGROUND_COLOR] = atol(value);
			else if (line.find(SPLASH_NB_LETTERS) != std::string::npos)
				mMapUnsigned[SPLASH_NB_LETTERS] = atol(value);
			else if (line.find(SPLASH_TIME) != std::string::npos)
				mMapUnsigned[SPLASH_TIME] = atol(value);

			// Traitement des parametres de type bool
			if (line.find(FONT_BOLD) != std::string::npos)
				mMapBool[FONT_BOLD] = value[0] != '0';
			else if (line.find(FONT_ITALIC) != std::string::npos)
				mMapBool[FONT_ITALIC] = value[0] != '0';
			else if (line.find(SPLASH_AUTO_INSERT) != std::string::npos)
				mMapBool[SPLASH_AUTO_INSERT] = value[0] != '0';
		}
		infile.close();
	}
}

Configuration::~Configuration()
{
}

bool Configuration::SaveIntoFile(std::string fichierConfig)
{
	// Ouverture avec ecrasement du fichier de config
	std::ofstream outfile(fichierConfig.c_str());
	if (!outfile.good()) return false; // Impossible de sauvegarder

	// Sauvegarde des parametres de type string
	outfile << "[string]" << std::endl;
	for (std::map<std::string, std::string>::const_iterator iter = mMapString.begin();
		iter != mMapString.end();
		++iter) {
		outfile << iter->first << " = " << iter->second << std::endl;
	}

	// Sauvegarde des parametres de type unsigned int
	outfile << std::endl << "[unsigned]" << std::endl;
	for (std::map<std::string, unsigned int>::const_iterator iter = mMapUnsigned.begin();
		iter != mMapUnsigned.end();
		++iter) {
		outfile << iter->first << " = " << iter->second << std::endl;
	}

	// Sauvegardes des parametres de type bool
	outfile << std::endl << "[bool]" << std::endl;
	for (std::map<std::string, bool>::const_iterator iter = mMapBool.begin();
		iter != mMapBool.end();
		++iter) {
		outfile << iter->first << " = " << iter->second << std::endl;
	}

	// Sauvegarde terminee
	outfile.close();

	return true;
}

std::string Configuration::GetStringParam(std::string paramName)
{
	std::string retour;

	// Recherche du pamametre
	std::map<std::string, std::string>::const_iterator iter;
	if ((iter = mMapString.find(paramName)) != mMapString.end()) // Existe
		retour = iter->second;

	return retour;
}

void Configuration::SetStringParam(std::string paramName, std::string paramValue)
{
	mMapString[paramName] = paramValue;
}

unsigned int Configuration::GetUnsignedIntParam(std::string paramName)
{
	unsigned int retour = 0;

	// Recherche du pamametre
	std::map<std::string, unsigned int>::const_iterator iter;
	if ((iter = mMapUnsigned.find(paramName)) != mMapUnsigned.end()) // Existe
		retour = iter->second;

	return retour;
}

void Configuration::SetUnsignedIntParam(std::string paramName, unsigned int paramValue)
{
	mMapUnsigned[paramName] = paramValue;
}

bool Configuration::GetBoolParam(std::string paramName)
{
	bool retour = false;

	// Recherche du pamametre
	std::map<std::string, bool>::const_iterator iter;
	if ((iter = mMapBool.find(paramName)) != mMapBool.end()) // Existe
		retour = iter->second;

	return retour;
}

void Configuration::SetBoolParam(std::string paramName, bool paramValue)
{
	mMapBool[paramName] = paramValue;
}

/* -------------------------------------------------------------------- */
/*                        PRIVATE                                       */
/* -------------------------------------------------------------------- */
/*------------------------------------------------- Fonctions privées   */
void Configuration::loadDefault()
{
	// Parametres string par defaut
	SetStringParam(FONT_TYPE, "NULL"); // Not defined

	// Parametres unsigned par defaut
	SetUnsignedIntParam(FONT_TEXT_COLOR, 0); // Not defined
	SetUnsignedIntParam(DIALOG_POS_X, 0); // Not defined
	SetUnsignedIntParam(DIALOG_POS_Y, 0); // Not defined
	SetUnsignedIntParam(DIALOG_WIDTH, 0); // Not defined
	SetUnsignedIntParam(DIALOG_HEIGHT, 0); // Not defined
	SetUnsignedIntParam(DIALOG_BACKGROUND_COLOR, 0); // Not defined
	SetUnsignedIntParam(SPLASH_NB_LETTERS, 0); // Affichage instantane
	SetUnsignedIntParam(SPLASH_TIME, 20000); // 20s

	// Parametres bool par defaut
	SetBoolParam(FONT_BOLD, false); // Pas de police en gras
	SetBoolParam(FONT_ITALIC, false); // Pas de police en italique
	SetBoolParam(SPLASH_AUTO_INSERT, false); // Pas d'auto insert
}