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

/* ------------------------------------------------------------ INCLUDE */
/* ---------------------------------------------------- Include système */
#include <windows.h> // CharUpper
#include <sstream> // Stream sur string
#include <fstream> // Gestion des flux de fichiers
#include <locale> // Test de majuscule etc
#include <iostream> // Flux d'entree/sortie
#include <algorithm> // std::sort

/* -------------------------------------------------- Include personnel */
#include "Dictionnaire.h"

/* -------------------------------------------------------------------- */
/*                        PRIVE                                         */
/* --------------------------------------------------------- Constantes */
#define MAX_WORD_LENGHT 255 // Definit la taille maximale d'un mot

/* -------------------------------------------------------------- Types */
/**
 * @brief Structure de trie pour les mots ponderes d'une liste
 */
struct sort_mot_inverted
{
  bool operator()(const MotPondere& lm, const MotPondere& rm)
  {
	  return lm.poids > rm.poids; // Tri decroissant
  }
};

/*--------------------------------------------------- Fonctions privees */
void Dictionnaire::trierDictionnaire()
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;   

	// Pour tous les dictionnaire de niveau 0
	for (iterNiveau0=dictionnaire.begin();
		iterNiveau0!=dictionnaire.end();
		++iterNiveau0) {
		// Pour tous les dictionnaire de niveau 1
		for (iterNiveau1=iterNiveau0->second.begin();
			iterNiveau1!=iterNiveau0->second.end();
			iterNiveau1++) {
			// Pour tous les dictionnaire de niveau 2
			for (iterNiveau2=iterNiveau1->second.begin();
				iterNiveau2!=iterNiveau1->second.end();
				iterNiveau2++) {
				std::sort(iterNiveau2->second.begin()
					, iterNiveau2->second.end()
					, sort_mot_inverted());
			}
		}
	}
}

void Dictionnaire::filtrerVecteurMots(std::string & keyWords
	, std::deque<MotPondere> & motsPonderes
	, std::vector<std::string> & results
	, unsigned int & maxResults
	, bool ignoreKeyWords)
{
	for (std::deque <MotPondere>::const_iterator iter = motsPonderes.begin();
		iter!=motsPonderes.end();
		++iter) {
		if (maxResults>0) { // On peut encore fournir des mots
			if (iter->mot.find(keyWords,0) != std::string::npos) {
				// Le mot commence bien pas la meme racine
				if ((ignoreKeyWords == false) || (iter->mot.compare(keyWords) != 0)) { 
					// Insere pas le mot recherche
					results.push_back(iter->mot);
					maxResults--;
				}
			}
		} 
		else // Maximum de mots atteint
			break;
	}
}

void Dictionnaire::adapterVecteurMots(std::string & keyWords
	, std::vector<std::string> & results)
{
	// Type de caractere
	std::locale loc("French_France");
	
	// Si la premiere lettre est une majuscule
	if (isupper(keyWords[0], loc)){
		// Contient le mot majuscule
		char motTemp[MAX_WORD_LENGHT+1];

		// Si plus de deux lettres ET deuxieme lettre en majuscule
		if ((keyWords.length() > 1) && (isupper(keyWords[1], loc)))	{ 
			// Pour tous les mots on les mets en majuscule
			for (unsigned int i=0; i<results.size(); i++ ) {
				if (results[i].length() < MAX_WORD_LENGHT) {
					strcpy(motTemp, results[i].c_str());
					CharUpper(motTemp); //_strupr( motTemp );
					results[i] = motTemp;
				}
				else
					continue;
			}
		}
		else { // Seulement la premiere lettre des mots a augmenter
			// Pour tous les mots on met en majuscule la premiere lettre
			for (unsigned int i=0; i<results.size(); i++) {
				if (results[i].length() < MAX_WORD_LENGHT) {
					strcpy(motTemp, results[i].c_str());
					motTemp[1] = '\0'; // Accelere le traitement
					CharUpper(motTemp); //_strupr( motTemp );
					results[i][0] = motTemp[0];
				}
				else
					continue;
			}
		}
	}
}

/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/*------------------------------------------------- Fonctions publiques */
Dictionnaire::Dictionnaire()
{
}

Dictionnaire::~Dictionnaire()
{
}

bool Dictionnaire::LoadFromFile(char * file, bool checkExisteMot)
{
	// Creation d'un flux de lecture sur le fichier
	std::ifstream infile(file, std::ios::binary);
	if (!infile.good())	return false;

	// Tant que on a des lignes a lire
	std::string line;
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);

		// Lecture des octets du mots et de sa ponderation
		MotPondere mot;
		char escape;
		iss >> mot.poids >> escape >> mot.mot;

		// Ignore les mots deja existant
		if (checkExisteMot)
			if (ExisteMot(mot.mot))
				continue;

		// Ignore si plus petit que 3 caracteres
		if (mot.mot.length() < 3)
			continue;

		// Sauvegarde du mot pondere dans le dictionnaire
		dictionnaire[mot.mot[0]][mot.mot[1]][mot.mot[2]].push_back(mot);
	}

	// Trie du dictionnaire par poid
	trierDictionnaire();

	return true;
}

bool Dictionnaire::SaveIntoFile(char * file)
{
	// Creation du flux d'ecriture avec ecrasement
	std::ofstream outfile(file, std::ios::binary);
	if (outfile.good()) {
		// Iterateur sur notre dictionnaire de niveau 0
		DictionnaireNiveau0::iterator iterNiveau0;
		// Iterateur sur notre dictionnaire de niveau 1
		DictionnaireNiveau1::iterator iterNiveau1;
		// Iterateur sur notre dictionnaire de niveau 2
		DictionnaireNiveau2::iterator iterNiveau2;   

		// Pour tous les dictionnaire de niveau 0
		for (iterNiveau0=dictionnaire.begin();
			iterNiveau0!=dictionnaire.end();
			++iterNiveau0) {
			// Pour tous les dictionnaire de niveau 1
			for (iterNiveau1=iterNiveau0->second.begin();
				iterNiveau1!=iterNiveau0->second.end();
				iterNiveau1++) {
				// Pour tous les dictionnaire de niveau 2
				for (iterNiveau2=iterNiveau1->second.begin();
					iterNiveau2!=iterNiveau1->second.end();
					iterNiveau2++) {
					// Pour tous les mot
					for (std::deque<MotPondere>::const_iterator iter = iterNiveau2->second.begin();
						iter != iterNiveau2->second.end();
						++iter) {
							outfile << iter->poids << ":" << iter->mot << std::endl;
					}
				}
			}
		}
		outfile.close();
	}
	
	return true;
}

unsigned int Dictionnaire::FindWordsAtAllCost(std::vector<std::string> & results
	, unsigned int maxResults
	, std::string keyWords)
{
	// Recherche rapide
	maxResults -= FindWords(results, maxResults, keyWords);

	// Type de caractere
	std::locale loc( "French_France" );
	
	// Gestion de la premiere lettre si aucun resultat et majuscule
	if((maxResults > 0) && isupper(keyWords[0],loc)) {
		std::string rechercheTemp = keyWords;
		if ((keyWords[0] == 'A') ||
			(keyWords[0] == 'U') ||
			(keyWords[0] == 'C') ||
			(keyWords[0] == 'E')) {
			switch (keyWords[0]) {
				case 'A': rechercheTemp[0] = 'À'; break;
				case 'U': rechercheTemp[0] = 'Ù'; break;
				case 'C': rechercheTemp[0] = 'Ç'; break;
				case 'E': rechercheTemp[0] = 'É'; break;
			}
			maxResults -= FindWords(results, maxResults, rechercheTemp);
			// On teste encore une autre possibilite si la lettre est E
			if ((keyWords[0] == 'E') && (maxResults > 0)) {
				rechercheTemp[0] = 'È';
				maxResults -= FindWords(results, maxResults, rechercheTemp);
			}
		}

		// Gestion de la deuxime lettre si aucun resultat et majuscule
		if((maxResults > 0) && isupper(keyWords[1],loc)) {
			rechercheTemp = keyWords;
			if ((keyWords[1] == 'A') ||
				(keyWords[1] == 'U') ||
				(keyWords[1] == 'C') ||
				(keyWords[1] == 'E')) {
				switch (keyWords[1]) {
					case 'A': rechercheTemp[1] = 'À'; break;
					case 'U': rechercheTemp[1] = 'Ù'; break;
					case 'C': rechercheTemp[1] = 'Ç'; break;
					case 'E': rechercheTemp[1] = 'É'; break;
				}
				maxResults -= FindWords(results, maxResults, rechercheTemp);
				// On teste encore une autre possibilite si la lettre est E
				if ((keyWords[1] == 'E') && (maxResults > 0)) {
					rechercheTemp[1] = 'È';
					maxResults -= FindWords(results, maxResults, rechercheTemp);
				}
			}
		}
	}

	return (unsigned int)results.size();
}

unsigned int Dictionnaire::FindWords(std::vector<std::string> & results
	, unsigned int maxResults
	, std::string keyWords
	, bool ignoreKeyWords)
{
	// Si aucune lettre fournie on ignore la demande
	if (keyWords.length() == 0)	return 0;

	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;   

	// Conversion en minuscule de keyWords
	char keyWordsTemp[MAX_WORD_LENGHT+1];
	std::string keyWordsOld = keyWords; // Sauvegarde pour l'adaptation
	if (keyWords.length() < MAX_WORD_LENGHT) { // Conversion possible
		strcpy(keyWordsTemp, keyWords.c_str());
		CharLower(keyWordsTemp); //_strlwr( keyWordsTemp );
		keyWords = keyWordsTemp;
	}
	else return false;

	// Si une seule lettre de fournie
	if (keyWords.length() == 1) {
		// On cherche les mots qui commence par la meme lettre
		iterNiveau0 = dictionnaire.find(keyWords[0]);
		if (iterNiveau0 == dictionnaire.end()) // Si on a pas trouve on retourne 0
			return 0;

		// Pour tous les dictionnaire de niveau 1
		for (iterNiveau1=iterNiveau0->second.begin();
		iterNiveau1!=iterNiveau0->second.end();
		iterNiveau1++) {
			// Pour tous les dictionnaire de niveau 2
			for (iterNiveau2=iterNiveau1->second.begin();
			iterNiveau2!=iterNiveau1->second.end();
			iterNiveau2++) {
				// On a notre vecteur de mots terminaux que l'on filtre
				filtrerVecteurMots(keyWords
					, iterNiveau2->second
					, results
					, maxResults
					, ignoreKeyWords);

				// Si on a remplit le nombre des possibles on quitte
				if (maxResults==0) {
					adapterVecteurMots(keyWordsOld,results); // Adaptation du resultat
					return (unsigned int)results.size(); // Taille du vecteur
				}
			}
		}
	}

	// Si deux lettres de fournie
	if (keyWords.length() == 2) {
		// On cherche les mots qui commence par la meme lettre
		iterNiveau0 = dictionnaire.find(keyWords[0]);
		if (iterNiveau0 == dictionnaire.end()) // Si on a pas trouve on retourne 0
			return 0;

		// On cherche les mots qui ont la meme deuxieme lettre
		iterNiveau1 = iterNiveau0->second.find(keyWords[1]);
		if (iterNiveau1 == iterNiveau0->second.end()) //Si on a pas trouve on retourne 0
			return 0;

		// Pour tous les dictionnaire de niveau 2
		for (iterNiveau2=iterNiveau1->second.begin();
		iterNiveau2!=iterNiveau1->second.end();
		iterNiveau2++) {
			// On a notre vecteur de mots terminaux que l'on filtre
			filtrerVecteurMots(keyWords
				, iterNiveau2->second
				, results
				, maxResults
				, ignoreKeyWords);

			// Si on a remplit le nombre des possibles on quitte
			if (maxResults==0) {
				adapterVecteurMots(keyWordsOld,results); // Adaptation du resultat
				return (unsigned int)results.size(); // Taille du vecteur
			}
		}
	}

	// Si trois lettre de fournies (Parfait)
	if (keyWords.length() >= 3)	{
		// On cherche les mots qui commence par la meme lettre
		iterNiveau0 = dictionnaire.find(keyWords[0]);
		if (iterNiveau0 == dictionnaire.end()) //Si on a pas trouve on retourne 0
			return 0;

		// On cherche les mots qui ont la meme deuxieme lettre
		iterNiveau1 = iterNiveau0->second.find(keyWords[1]);
		if (iterNiveau1 == iterNiveau0->second.end()) //Si on a pas trouve on retourne 0
			return 0;

		// On cherche les mots qui ont la meme troisieme lettre
		iterNiveau2 = iterNiveau1->second.find(keyWords[2]);
		if (iterNiveau2 == iterNiveau1->second.end()) //Si on a pas trouve on retourne 0
			return 0;

		// On a notre vecteur de mots terminaux que l'on filtre
		filtrerVecteurMots(keyWords
			, iterNiveau2->second
			, results
			, maxResults
			, ignoreKeyWords);
	}

	// Adaptation du resultat
	adapterVecteurMots(keyWordsOld, results); 

	// Taille du vecteur
	return (unsigned int)results.size();
}

void Dictionnaire::IncrementerPoids(std::string mot)
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;  

	// Conversion en minuscule de mot
	char motTemp[MAX_WORD_LENGHT+1];
	if (mot.length() < MAX_WORD_LENGHT) { // Conversion possible
		strcpy(motTemp,mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return;

	// Si trois lettre de fournies (Parfait)
	if (mot.length() >= 3) {
		// On cherche les mots qui commence par la meme lettre
		iterNiveau0 = dictionnaire.find(mot[0]);
		if (iterNiveau0 == dictionnaire.end()) //Si on a pas trouve on retourne
			return;

		// On cherche les mots qui ont la meme deuxieme lettre
		iterNiveau1 = iterNiveau0->second.find(mot[1]);
		if (iterNiveau1 == iterNiveau0->second.end()) //Si on a pas trouve on retourne
			return;

		// On cherche les mots qui ont la meme troisieme lettre
		iterNiveau2 = iterNiveau1->second.find(mot[2]);
		if (iterNiveau2 == iterNiveau1->second.end()) //Si on a pas trouve on retourne
			return;

		for (std::deque<MotPondere>::iterator iter = iterNiveau2->second.begin();
		iter != iterNiveau2->second.end();
		++iter) {
			// On a trouve le mot
			if (iter->mot.compare(mot) == 0) { 
				iter->poids++; // Incremente le poids
				std::sort(iterNiveau2->second.begin()
					, iterNiveau2->second.end()
					, sort_mot_inverted());
				break;
			}
		}
	}
}

void Dictionnaire::SupprimerMot(std::string mot)
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;  

	// Conversion en minuscule de mot
	char motTemp[MAX_WORD_LENGHT+1];
	if (mot.length() < MAX_WORD_LENGHT) { // Conversion possible
		strcpy(motTemp,mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return;

	// Si trois lettre de fournies (Parfait)
	if (mot.length() >= 3) {
		// On cherche les mots qui commence par la meme lettre
		iterNiveau0 = dictionnaire.find(mot[0]);
		if (iterNiveau0 == dictionnaire.end()) //Si on a pas trouve on retourne
			return;

		// On cherche les mots qui ont la meme deuxieme lettre
		iterNiveau1 = iterNiveau0->second.find(mot[1]);
		if (iterNiveau1 == iterNiveau0->second.end()) //Si on a pas trouve on retourne
			return;

		// On cherche les mots qui ont la meme troisieme lettre
		iterNiveau2 = iterNiveau1->second.find(mot[2]);
		if (iterNiveau2 == iterNiveau1->second.end()) //Si on a pas trouve on retourne
			return;

		for (std::deque<MotPondere>::iterator iter = iterNiveau2->second.begin();
		iter != iterNiveau2->second.end();
		++iter) {
			// On a trouve le mot
			if (iter->mot.compare(mot) == 0) { 
				iterNiveau2->second.erase(iter); // Supprime le mot
				break;
			}
		}
	}
}

void Dictionnaire::SupprimerMots()
{
	dictionnaire.clear();
}

bool Dictionnaire::ExisteMot(std::string mot)
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;  

	// Conversion en minuscule de mot
	char motTemp[MAX_WORD_LENGHT+1];
	if (mot.length()<MAX_WORD_LENGHT) { // Conversion possible
		strcpy(motTemp,mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return false;

	// Si trois lettre de fournies (Parfait)
	if (mot.length() >= 3) {
		// On cherche les mots qui commence par la meme lettre
		iterNiveau0 = dictionnaire.find(mot[0]);
		if (iterNiveau0 == dictionnaire.end()) //Si on a pas trouve on retourne
			return false;

		// On cherche les mots qui ont la meme deuxieme lettre
		iterNiveau1 = iterNiveau0->second.find(mot[1]);
		if (iterNiveau1 == iterNiveau0->second.end()) //Si on a pas trouve on retourne
			return false;

		// On cherche les mots qui ont la meme troisieme lettre
		iterNiveau2 = iterNiveau1->second.find(mot[2]);
		if (iterNiveau2 == iterNiveau1->second.end()) //Si on a pas trouve on retourne
			return false;

		for (std::deque<MotPondere>::iterator iter = iterNiveau2->second.begin();
		iter != iterNiveau2->second.end();
		++iter) {
			// On a trouve le mot
			if (iter->mot.compare(mot) == 0) { 
				return true; // Le mot exite
			}
		}
	}

	return false;
}

void Dictionnaire::AjouterMot(std::string mot)
{
	if (mot.length() >= 3) {
		// Conversion en minuscule de mot
		char motTemp[MAX_WORD_LENGHT+1];
		if (mot.length()<MAX_WORD_LENGHT) { // Conversion possible
			strcpy(motTemp,mot.c_str());
			CharLower(motTemp);
			mot = motTemp;
		}
		else return;

		MotPondere motPond;
		motPond.poids = 0;
		motPond.mot = mot;

		// Sauvegarde du mot pondere dans le dictionnaire
		dictionnaire[motPond.mot[0]][motPond.mot[1]][motPond.mot[2]].push_back(motPond);
	}
}

void Dictionnaire::ResetPoids(std::string mot)
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;  

	// Conversion en minuscule de mot
	char motTemp[MAX_WORD_LENGHT+1];
	if (mot.length() < MAX_WORD_LENGHT) { // Conversion possible
		strcpy(motTemp,mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return;

	// Si trois lettre de fournies (Parfait)
	if (mot.length() >= 3) {
		// On cherche les mots qui commence par la meme lettre
		iterNiveau0 = dictionnaire.find(mot[0]);
		if (iterNiveau0 == dictionnaire.end()) //Si on a pas trouve on retourne
			return;

		// On cherche les mots qui ont la meme deuxieme lettre
		iterNiveau1 = iterNiveau0->second.find(mot[1]);
		if (iterNiveau1 == iterNiveau0->second.end()) //Si on a pas trouve on retourne
			return;

		// On cherche les mots qui ont la meme troisieme lettre
		iterNiveau2 = iterNiveau1->second.find(mot[2]);
		if (iterNiveau2 == iterNiveau1->second.end()) //Si on a pas trouve on retourne
			return;

		for (std::deque<MotPondere>::iterator iter = iterNiveau2->second.begin();
		iter != iterNiveau2->second.end();
		++iter) {
			// On a trouve le mot
			if (iter->mot.compare(mot) == 0) { 
				iter->poids = 0; // RAZ du poids
				std::sort(iterNiveau2->second.begin()
					, iterNiveau2->second.end()
					, sort_mot_inverted());
				break;
			}
		}
	}
}

void Dictionnaire::ResetPoids()
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;   

	// Pour tous les dictionnaire de niveau 0
	for (iterNiveau0 = dictionnaire.begin();
	iterNiveau0 != dictionnaire.end();
	++iterNiveau0) {
		// Pour tous les dictionnaire de niveau 1
		for (iterNiveau1 = iterNiveau0->second.begin();
		iterNiveau1 != iterNiveau0->second.end();
		++iterNiveau1) {
			// Pour tous les dictionnaire de niveau 2
			for (iterNiveau2 = iterNiveau1->second.begin();
			iterNiveau2 != iterNiveau1->second.end();
			++iterNiveau2) {
				// Pour tous les mot
				for (std::deque<MotPondere>::iterator iter = iterNiveau2->second.begin();
				iter != iterNiveau2->second.end();
				++iter) {
					iter->poids = 0;
				}
			}
		}
	}

	// Il faut retrier le dictionnaire
	trierDictionnaire();
}