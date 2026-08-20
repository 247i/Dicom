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

static int distanceLev(wstring str1, wstring str2) {
	int len1 = str1.length();
	int len2 = str2.length();
	int cost = 0;

	// On tronque le mot du dictionnaire à la meme taille que le début de mot tapé
	// pour ne pas fausser la distance par des "ajouts" de lettre en fin de mot
	if (len1 >= len2)
	{
		wprintf(L"%.*s", len1 - 1, str1);
		len1 = str1.length();
	}

	// tab is a table with lenStr1+1 rows and lenStr2+1 columns
	int tab[MAX_WORD_LENGHT][MAX_WORD_LENGHT];

	// initialisation du tableau
	for (int i = 0; i <= len1; i++){ tab[i][0] = i; }

	for (int j = 0; j <= len2; j++){ tab[0][j] = j; }

	for (int i = 1; i <= len1; i++)
	{
		for (int j = 1; j <= len2; j++)
		{
			cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;

			tab[i][j] = min((tab[i - 1][j] + 1),     // deletion
				min((tab[i][j - 1] + 1),     // insertion
				(tab[i - 1][j - 1] + cost))  // substitution
				);

			if ((i > 1) && (j > 1) && (str1[i - 1] == str2[j - 2]) && (str1[i - 2] == str2[j - 1]))
			{
				tab[i][j] = min(
					tab[i][j],
					(tab[i - 2][j - 2] + cost)   // transposition
					);
			}
		}
	}
	return tab[len1][len2];
}

/*--------------------------------------------------- Fonctions privees */
void Dictionnaire::trierDictionnaire()
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iter0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iter1;

	DictionnaireNiveau2::iterator iter2;

	// TRI DICO ONE CHAR
	for (iter2 = dico1.begin();
		iter2 != dico1.end();
		iter2++) {
		std::sort(iter2->second.begin(), iter2->second.end(), sort_mot_inverted());
	}

	// TRI DICO TWO CHAR
	for (iter1 = dico2.begin();
		iter1 != dico2.end();
		iter1++) {
		for (iter2 = iter1->second.begin();
			iter2 != iter1->second.end();
			iter2++) {
			std::sort(iter2->second.begin(), iter2->second.end(), sort_mot_inverted());
		}
	}

	// TRI DICO THREE CHAR AND MORE...
	for (iter0=dico.begin();
		iter0!=dico.end();
		++iter0) {
		// Pour tous les dictionnaire de niveau 1
		for (iter1=iter0->second.begin();
			iter1!=iter0->second.end();
			iter1++) {
			for (iter2 = iter1->second.begin();
				iter2 != iter1->second.end();
				iter2++) {
				std::sort(iter2->second.begin(), iter2->second.end(), sort_mot_inverted());
			}
		}
	}
}

void Dictionnaire::filtrerVecteurMots(wstring & keyWords
	, std::deque<MotPondere> & motsPonderes
	, std::vector<wstring> & results
	, unsigned int & maxResults
	, bool ignoreKeyWords)
{
	for (std::deque <MotPondere>::const_iterator iter = motsPonderes.begin();
		iter!=motsPonderes.end();
		++iter) {
		if (maxResults>0) { // On peut encore fournir des mots
			if (iter->mot.find(keyWords) != wstring::npos /*|| distanceLev(iter->mot, keyWords) <= 3*/)/*iter->mot.find(keyWords,0) != std::string::npos*/ {
				// Le mot commence bien pas la meme racine
				if ((ignoreKeyWords == false) || (iter->mot.compare(keyWords) != 0)) { 
					// Insere pas le mot recherche
					results.push_back(iter->mot);
					maxResults--;
				}
			}
		} 
		else // Maximum de mots atteint
			return;
	}
	for (std::deque <MotPondere>::const_iterator iter = motsPonderes.begin();
		iter != motsPonderes.end();
		++iter) {
		if (maxResults > 0 && std::find(results.begin(), results.end(), iter->mot) == results.end()) { // On peut encore fournir des mots
			if (distanceLev(iter->mot, keyWords) <= 3) {
				// Le mot commence bien pas la meme racine
				if ((ignoreKeyWords == false) || (iter->mot.compare(keyWords) != 0)) {
					// Insere pas le mot recherche
					results.push_back(iter->mot);
					maxResults--;
				}
			}
		}
		else // Maximum de mots atteint
			return;
	}
	
}

void Dictionnaire::adapterVecteurMots(wstring & keyWords
	, std::vector<wstring> & results)
{
	// Type de caractere
	std::locale loc("French_France");

	// Si la premiere lettre est une majuscule
	if (isupper(keyWords[0], loc)){
		// Contient le mot majuscule
		wchar_t motTemp[MAX_WORD_LENGHT+1];

		// Si plus de deux lettres ET deuxieme lettre en majuscule
		if ((keyWords.length() > 1) && (isupper(keyWords[1], loc)))	{ 
			// Pour tous les mots on les mets en majuscule
			for (unsigned int i=0; i<results.size(); i++ ) {
				if (results[i].length() < MAX_WORD_LENGHT) {
					wcscpy_s(motTemp, results[i].c_str());
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
					wcscpy_s(motTemp, results[i].c_str());
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

bool Dictionnaire::LoadFromFile(wstring file, bool newDico)
{

	SupprimerMots();

	// Creation d'un flux de lecture sur le fichier
	wchar_t buf[256];
	wsprintf(buf, L"%s%s%s\0", PATH_TO_DICO, file.c_str(), DIC);
	wstring filepath = newDico ? file : buf;
	std::wifstream infile(filepath, std::ios::binary);
	if (!infile.good())	return false;

	// Tant que on a des lignes a lire
	wstring line;
	while (getline(infile, line))
	{

		// Lecture des octets du mots et de sa ponderation
		std::wistringstream iss(line);

		// Lecture des octets du mots et de sa ponderation
		MotPondere mot;
		wchar_t escape;
		iss >> mot.poids >> escape >> mot.mot;

		// Ignore si plus petit que 3 caracteres
		if (mot.mot.length() < 3)
			continue;

		bool exist = false;
		DictionnaireNiveau0::iterator iter0;
		DictionnaireNiveau1::iterator iter1;
		DictionnaireNiveau2::iterator iter2;
		std::deque<MotPondere>::iterator iterMot;
		
		iter0 = dico.find(mot.mot[0]);
		if (iter0 != dico.end()) {
			iter1 = iter0->second.find(mot.mot[1]);
			if (iter1 != iter0->second.end()) {
				iter2 = iter1->second.find(mot.mot[2]);
				if (iter2 != iter1->second.end()) {
					for (iterMot = iter2->second.begin();
						iterMot != iter2->second.end();
						iterMot++) {
						if (iterMot->mot.compare(mot.mot) == 0) {
							exist = true;
							if (iterMot->poids < mot.poids) {
								dico1[mot.mot[0]].erase(iterMot);
								dico2[mot.mot[0]][mot.mot[1]].erase(iterMot);
								dico[mot.mot[0]][mot.mot[1]][mot.mot[2]].erase(iterMot);
								exist = false;
							}
							break;
						}
					}
				}
			}
		}
		
		// Sauvegarde du mot pondere dans les dictionnaires
		if (!exist) {
			dico1[mot.mot[0]].push_back(mot);
			dico2[mot.mot[0]][mot.mot[1]].push_back(mot);
			dico[mot.mot[0]][mot.mot[1]][mot.mot[2]].push_back(mot);
		}
	}

	// Trie du dictionnaire par poid
	trierDictionnaire();

	return true;
}

std::deque<MotPondere> Dictionnaire::GetAllWords() {
	std::deque<MotPondere> toReturn;
	DictionnaireNiveau2::iterator iter;
	std::deque<MotPondere>::iterator iterMot;
	for (iter = dico1.begin();
		iter != dico1.end();
		iter++) {
		for (iterMot = iter->second.begin();
			iterMot != iter->second.end();
			iterMot++) {
			MotPondere mp;
			mp.mot = iterMot->mot;
			mp.poids = iterMot->poids;
			toReturn.push_back(mp);
		}
	}
	return toReturn;
}

bool Dictionnaire::SaveIntoFile(wstring file) {
	wchar_t filepath[256];
	wsprintf(filepath, L"%s%s%s\0", PATH_TO_DICO, file.c_str(), DIC);
	std::wofstream outfile(filepath, std::ios::binary);
	if (outfile.good()) {
		DictionnaireNiveau2::iterator iter;
		std::deque<MotPondere>::iterator iterMot;
		for (iter = dico1.begin();
			iter != dico1.end();
			iter++) {
			for (iterMot = iter->second.begin();
				iterMot != iter->second.end();
				iterMot++) {
				outfile << iterMot->poids << ":" << iterMot->mot << std::endl;
			}
		}
		outfile.close();
	}
	return true;
}

unsigned int Dictionnaire::FindWordsAtAllCost(std::vector<wstring> & results
	, unsigned int maxResults
	, wstring keyWords)
{
	// Recherche rapide
	maxResults -= FindWords(results, maxResults, keyWords);

	// Type de caractere
	std::locale loc( "French_France" );
	
	// Gestion de la premiere lettre si aucun resultat et majuscule
	if((maxResults > 0) && isupper(keyWords[0],loc)) {
		wstring rechercheTemp = keyWords;
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

unsigned int Dictionnaire::FindWords(std::vector<wstring> & results
	, unsigned int maxResults
	, wstring keyWords
	, bool ignoreKeyWords)
{
	// Si aucune lettre fournie on ignore la demande
	if (keyWords.length() == 0)	return 0;

		//TCHAR rou[256];
		//sprintf(rou, "return iterniveau2 %s %c %c %c %c\n", keyWords.c_str(), keyWords[0], keyWords.size() > 1 ? keyWords[1] : '~', keyWords.size() > 2 ? keyWords[2] : '~', keyWords.size() > 3 ? keyWords[3] : '~');
		//OutputDebugString(rou);
	

	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iter0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iter1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iter2; 

	// Conversion en minuscule de keyWords
	wchar_t keyWordsTemp[MAX_WORD_LENGHT+1];
	wstring keyWordsOld = keyWords; // Sauvegarde pour l'adaptation
	if (keyWords.length() < MAX_WORD_LENGHT) { // Conversion possible
		wsprintf(keyWordsTemp, keyWords.c_str());
		CharLower(keyWordsTemp); //_strlwr( keyWordsTemp );
		keyWords = keyWordsTemp;
	}
	else return false;

	// Si une seule lettre de fournie
	if (keyWords.length() == 1) {
		// On cherche les mots qui commence par la meme lettre
		iter2 = dico1.find(keyWords[0]);
		if (iter2 == dico1.end()) // Si on a pas trouve on retourne 0
			return 0;

		filtrerVecteurMots(keyWords
			, iter2->second
			, results
			, maxResults
			, ignoreKeyWords);
			
		if (maxResults==0) {
			adapterVecteurMots(keyWordsOld,results); // Adaptation du resultat
			return (unsigned int)results.size(); // Taille du vecteur
		}
	}

	// Si deux lettres de fournie
	if (keyWords.length() == 2) {
		// On cherche les mots qui commence par la meme lettre
		iter1 = dico2.find(keyWords[0]);
		if (iter1 == dico2.end()) // Si on a pas trouve on retourne 0
			return 0;

		// On cherche les mots qui ont la meme deuxieme lettre
		iter2 = iter1->second.find(keyWords[1]);
		if (iter2 == iter1->second.end()) //Si on a pas trouve on retourne 0
			return 0;

		// On a notre vecteur de mots terminaux que l'on filtre
		filtrerVecteurMots(keyWords
			, iter2->second
			, results
			, maxResults
			, ignoreKeyWords);

			// Si on a remplit le nombre des possibles on quitte
			if (maxResults==0) {
				adapterVecteurMots(keyWordsOld,results); // Adaptation du resultat
				return (unsigned int)results.size(); // Taille du vecteur
			}
		}
	

	// Si trois lettre de fournies (Parfait)
	if (keyWords.length() >= 3)	{
		// On cherche les mots qui commence par la meme lettre
		iter0 = dico.find(keyWords[0]);
		if (iter0 == dico.end()) {//Si on a pas trouve on retourne 0
			return 0;
		}
		// On cherche les mots qui ont la meme deuxieme lettre
		iter1 = iter0->second.find(keyWords[1]);
		if (iter1 == iter0->second.end()) { //Si on a pas trouve on retourne 0
			return 0;
		}
		// On cherche les mots qui ont la meme troisieme lettre
		iter2 = iter1->second.find(keyWords[2]);
		if (iter2 == iter1->second.end()) { //Si on a pas trouve on retourne 0
			return 0;
		}
		// On a notre vecteur de mots terminaux que l'on filtre
		filtrerVecteurMots(keyWords
			, iter2->second
			, results
			, maxResults
			, ignoreKeyWords);
	}

	// Adaptation du resultat
	adapterVecteurMots(keyWordsOld, results); 

	// Taille du vecteur
	return (unsigned int)results.size();
}

unsigned int Dictionnaire::GetPond(wstring mot) {
	DictionnaireNiveau0::iterator iter0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iter1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iter2;

	// Conversion en minuscule de mot
	wchar_t motTemp[MAX_WORD_LENGHT + 1];
	if (mot.length() < MAX_WORD_LENGHT) { // Conversion possible
		wcscpy_s(motTemp, mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return -1;

	// On cherche les mots qui commence par la meme lettre
	iter0 = dico.find(mot[0]);
	if (iter0 == dico.end()) //Si on a pas trouve on retourne
		return -1;

	// On cherche les mots qui ont la meme deuxieme lettre
	iter1 = iter0->second.find(mot[1]);
	if (iter1 == iter0->second.end()) //Si on a pas trouve on retourne
		return -1;

	// On cherche les mots qui ont la meme troisieme lettre
	iter2 = iter1->second.find(mot[2]);
	if (iter2 == iter1->second.end()) //Si on a pas trouve on retourne
		return -1;

	for (std::deque<MotPondere>::iterator iter = iter2->second.begin();
		iter != iter2->second.end();
		++iter) {
		// On a trouve le mot
		if (iter->mot.compare(mot) == 0) {
			return iter->poids;
		}
	}
	return -1;
}

void Dictionnaire::SetPond(wstring mot, unsigned int pond) {
	DictionnaireNiveau0::iterator iter0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iter1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iter2;

	// Conversion en minuscule de mot
	wchar_t motTemp[MAX_WORD_LENGHT + 1];
	if (mot.length() < MAX_WORD_LENGHT) { // Conversion possible
		wcscpy_s(motTemp, mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return;

	/* MAJ DICO */
	// On cherche les mots qui commence par la meme lettre
	iter0 = dico.find(mot[0]);
	if (iter0 == dico.end()) //Si on a pas trouve on retourne
		return;

	// On cherche les mots qui ont la meme deuxieme lettre
	iter1 = iter0->second.find(mot[1]);
	if (iter1 == iter0->second.end()) //Si on a pas trouve on retourne
		return;

	// On cherche les mots qui ont la meme troisieme lettre
	iter2 = iter1->second.find(mot[2]);
	if (iter2 == iter1->second.end()) //Si on a pas trouve on retourne
		return;

	for (std::deque<MotPondere>::iterator iter = iter2->second.begin();
		iter != iter2->second.end();
		++iter) {
		// On a trouve le mot
		if (iter->mot.compare(mot) == 0) {
			iter->poids = pond;
			trierDictionnaire();
			break;
		}
	}

	/* MAJ DICO2 */
	iter1 = dico2.find(mot[0]);
	iter2 = iter1->second.find(mot[1]);
	for (std::deque<MotPondere>::iterator iter = iter2->second.begin();
		iter != iter2->second.end();
		++iter) {
		if (iter->mot.compare(mot) == 0) {
			iter->poids = pond;
			trierDictionnaire();
			break;
		}
	}

	/* MAJ DICO1 */
	iter2 = dico1.find(mot[0]);
	for (std::deque<MotPondere>::iterator iter = iter2->second.begin();
		iter != iter2->second.end();
		++iter) {
		if (iter->mot.compare(mot) == 0) {
			iter->poids = pond;
			trierDictionnaire();
			break;
		}
	}
}


void Dictionnaire::IncrementerPoids(wstring mot)
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iter0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iter1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iter2;  

	// Conversion en minuscule de mot
	wchar_t motTemp[MAX_WORD_LENGHT+1];
	if (mot.length() < MAX_WORD_LENGHT) { // Conversion possible
		wcscpy_s(motTemp, mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return;

	iter2 = dico1.find(mot[0]);
	if (iter2 == dico1.end())
		return;

	for (std::deque<MotPondere>::iterator iter = iter2->second.begin();
		iter != iter2->second.end();
		++iter) {
		if (iter->mot.compare(mot) == 0) {
			iter->poids++; // Incremente le poids
			std::sort(iter2->second.begin()
				, iter2->second.end()
				, sort_mot_inverted());
			break;
		}
	}

	// On cherche les mots qui ont la meme troisieme lettre
	iter1 = dico2.find(mot[0]);
	if (iter1 == dico2.end()) //Si on a pas trouve on retourne
		return;

	iter2 = iter1->second.find(mot[1]);
	if (iter2 == iter1->second.end())
		return;

	for (std::deque<MotPondere>::iterator iter = iter2->second.begin();
		iter != iter2->second.end();
		++iter) {
		// On a trouve le mot
		if (iter->mot.compare(mot) == 0) {
			iter->poids++; // Incremente le poids
			trierDictionnaire();
			break;
		}
	}

	// On cherche les mots qui commence par la meme lettre
	iter0 = dico.find(mot[0]);
	if (iter0 == dico.end()) //Si on a pas trouve on retourne
		return;

	// On cherche les mots qui ont la meme deuxieme lettre
	iter1 = iter0->second.find(mot[1]);
	if (iter1 == iter0->second.end()) //Si on a pas trouve on retourne
		return;

	// On cherche les mots qui ont la meme troisieme lettre
	iter2 = iter1->second.find(mot[2]);
	if (iter2 == iter1->second.end()) //Si on a pas trouve on retourne
		return;

	for (std::deque<MotPondere>::iterator iter = iter2->second.begin();
	iter != iter2->second.end();
	++iter) {
		// On a trouve le mot
		if (iter->mot.compare(mot) == 0) { 
			iter->poids++; // Incremente le poids
			std::sort(iter2->second.begin()
				, iter2->second.end()
				, sort_mot_inverted());
			break;
		}
	}		
	
}

void Dictionnaire::SupprimerMot(wstring mot)
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iter0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iter1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iter2;

	std::deque<MotPondere>::iterator iter;

	// Conversion en minuscule de mot
	wchar_t motTemp[MAX_WORD_LENGHT+1];
	if (mot.length() < MAX_WORD_LENGHT) { // Conversion possible
		wcscpy_s(motTemp, mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return;

	// SUPPRESSION DICO ONE CHAR
	iter2 = dico1.find(mot[0]);
	if (iter2 == dico1.end())
		return;
	for (iter = iter2->second.begin();
		iter != iter2->second.end();
		++iter) {
		if (iter->mot.compare(mot) == 0) {
			iter2->second.erase(iter);
			break;
		}
	}

	// SUPPRESSION DICO TWO CHAR
	iter1 = dico2.find(mot[0]);
	iter2 = iter1->second.find(mot[1]);
	for (iter = iter2->second.begin();
		iter != iter2->second.end();
		++iter) {
		if (iter->mot.compare(mot) == 0) {
			iter2->second.erase(iter);
			break;
		}
	}

	// SUPPRESSION DICO THREE CHAR AND MORE
	iter0 = dico.find(mot[0]);
	if (iter0 == dico.end()) //Si on a pas trouve on retourne
		return;
	// On cherche les mots qui ont la meme deuxieme lettre
	iter1 = iter0->second.find(mot[1]);
	if (iter1 == iter0->second.end()) //Si on a pas trouve on retourne
		return;
	// On cherche les mots qui ont la meme troisieme lettre
	iter2 = iter1->second.find(mot[2]);
	if (iter2 == iter1->second.end()) //Si on a pas trouve on retourne
		return;
	for (iter = iter2->second.begin();
	iter != iter2->second.end();
	++iter) {
		// On a trouve le mot
		if (iter->mot.compare(mot) == 0) { 
			iter2->second.erase(iter); // Supprime le mot
			break;
		}
	}

	
	
}

void Dictionnaire::SupprimerMots()
{
	dico.clear();
	dico1.clear();
	dico2.clear();
}

bool Dictionnaire::ExisteMot(wstring mot)
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;  

	// Conversion en minuscule de mot
	wchar_t motTemp[MAX_WORD_LENGHT+1];
	if (mot.length()<MAX_WORD_LENGHT) { // Conversion possible
		wcscpy_s(motTemp,mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return false;

	// Si trois lettre de fournies (Parfait)
	if (mot.length() >= 3) {
		// On cherche les mots qui commence par la meme lettre
		iterNiveau0 = dico.find(mot[0]);
		if (iterNiveau0 == dico.end()) //Si on a pas trouve on retourne
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

void Dictionnaire::AjouterMot(wstring mot, unsigned int poids)
{
	if (mot.length() >= 3) {
		// Conversion en minuscule de mot
		wchar_t motTemp[MAX_WORD_LENGHT+1];
		if (mot.length()<MAX_WORD_LENGHT) { // Conversion possible
			wcscpy_s(motTemp, mot.c_str());
			CharLower(motTemp);
			mot = motTemp;
		}
		else return;
		if (ExisteMot(mot)) return;
		MotPondere motPond;
		motPond.poids = poids;
		motPond.mot = mot;

		// Sauvegarde du mot pondere dans le dictionnaire
		dico1[motPond.mot[0]].push_back(motPond);
		dico2[motPond.mot[0]][motPond.mot[1]].push_back(motPond);
		dico[motPond.mot[0]][motPond.mot[1]][motPond.mot[2]].push_back(motPond);

		trierDictionnaire();
	}
}

void Dictionnaire::ResetPoids(wstring mot)
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;  

	std::deque<MotPondere>::iterator iter;

	// Conversion en minuscule de mot
	wchar_t motTemp[MAX_WORD_LENGHT+1];
	if (mot.length() < MAX_WORD_LENGHT) { // Conversion possible
		wcscpy_s(motTemp,mot.c_str());
		CharLower(motTemp);
		mot = motTemp;
	}
	else return;

	iterNiveau2 = dico1.find(mot[0]);
	if (iterNiveau2 == dico1.end()) //Si on a pas trouve on retourne
		return;
	for (iter = iterNiveau2->second.begin();
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

	// On cherche les mots qui ont la meme deuxieme lettre
	iterNiveau1 = iterNiveau0->second.find(mot[0]);
	if (iterNiveau1 == iterNiveau0->second.end()) //Si on a pas trouve on retourne
		return;
	// On cherche les mots qui ont la meme troisieme lettre
	iterNiveau2 = iterNiveau1->second.find(mot[1]);
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

	// On cherche les mots qui commence par la meme lettre
	iterNiveau0 = dico.find(mot[0]);
	if (iterNiveau0 == dico.end()) //Si on a pas trouve on retourne
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

void Dictionnaire::ResetPoids()
{
	// Iterateur sur notre dictionnaire de niveau 0
	DictionnaireNiveau0::iterator iterNiveau0;
	// Iterateur sur notre dictionnaire de niveau 1
	DictionnaireNiveau1::iterator iterNiveau1;
	// Iterateur sur notre dictionnaire de niveau 2
	DictionnaireNiveau2::iterator iterNiveau2;   

	std::deque<MotPondere>::iterator iter;

	for (iterNiveau2 = dico1.begin();
		iterNiveau2 != dico1.end();
		iterNiveau2++) {
		for (iter = iterNiveau2->second.begin();
			iter != iterNiveau2->second.end();
			++iter) {
			iter->poids = 0;
		}
	}

	// Pour tous les dictionnaire de niveau 1
	for (iterNiveau1 = dico2.begin();
		iterNiveau1 != dico2.end();
		++iterNiveau1) {
		// Pour tous les dictionnaire de niveau 2
		for (iterNiveau2 = iterNiveau1->second.begin();
			iterNiveau2 != iterNiveau1->second.end();
			++iterNiveau2) {
			// Pour tous les mot
			for (iter = iterNiveau2->second.begin();
				iter != iterNiveau2->second.end();
				++iter) {
				iter->poids = 0;
			}
		}
	}

	// Pour tous les dictionnaire de niveau 0
	for (iterNiveau0 = dico.begin();
	iterNiveau0 != dico.end();
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
				for (iter = iterNiveau2->second.begin();
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

BOOL Dictionnaire::DeleteDico(wstring dico) {
	wchar_t filepath[256];
	wsprintf(filepath, L"%s%s%s\0", PATH_TO_DICO, dico.c_str(), DIC);
	return DeleteFile(filepath);
}