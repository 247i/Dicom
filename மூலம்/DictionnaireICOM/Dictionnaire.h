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
/*					Dictionnaire.h - Header la classe Dictionnaire		*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* ------------------------------------------------------------ INCLUDE */
/* ---------------------------------------------------- Include système */
#include <string>
#include <vector>
#include <deque>
#include <map>

using namespace std;
/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/* -------------------------------------------------------------- Types */
#define DictionnaireNiveau2 std::map<wchar_t, std::deque<MotPondere>>
#define DictionnaireNiveau1 std::map<wchar_t, DictionnaireNiveau2 >
#define DictionnaireNiveau0 std::map<wchar_t, DictionnaireNiveau1 >

/* ------------------------------------------------- Variables globales */
#define PATH_TO_DICO L".\\Dictionnaires\\"
#define DIC L".dic"

/**
 * @brief Structure permettant de pondere un mot dans le dictionnaire.
 */
struct MotPondere
{
	unsigned int poids; // Plus le poids est grand, plus le mot est prioritaire
	wstring mot;
};

typedef enum Language
{
	FR, 
	EN
};

static int distanceLev(wstring str1, wstring str2);

class Dictionnaire
{
/* -------------------------------------------------------------------- */
/*                        PUBLIC                                        */
/* -------------------------------------------------------------------- */
public:
/*------------------------------------------------- Fonctions publiques */
	/**
	 * @brief Constructeur de la classe de gestion d'un dictionnaire.
	 */
	Dictionnaire();

	/**
	 * @brief Destructeur de la classe Dictionnaire.
	 * Il libere toute alocation dynamique.
	 */
	~Dictionnaire();


	/**
	 * @brief Charge un fichier dictionnaire a partir d'un fichier sur le
	 * disque dur.
	 * @param file Chemin d'acces au fichier a charger.
	 * @param checkExisteMot Indique s'il faut verifier la presense des mots du fichier
	 * dans le dictionnaire avant de les inserer.
     * @retval true Aucune erreur de chargement.
	 * @retval false Une erreur est survenue pendant le chargement.
	 * ATTENTION : Aucun mot du dictionnaire ne doit etre inferieur a 3 caracteres.
	*/
	bool LoadFromFile(wstring file, bool newDico = false);

	/**
	 * @brief Sauvegarde le contenu actuel du dictionnaire dans un fichier.
	 * @param file Chemin d'acces au fichier ou sauvegarder le dictionnaire.
	 * @retval true Aucune erreur de sauvegarde.
	 * @retval false Une erreur est survenue pendant la sauvegarde.
	 */
	bool SaveIntoFile(wstring file);

	/**
	 * @brief Recherche les mots pouvant completer le texte fournit en entree.
	 * Une erreur d'accent est autorisee sur la premiere et deuxieme lettre.
	 * Cette appel peut prendre beaucoup plus de temps que FindWords.
	 * @param results Vecteur de chaines de caracteres dans lequel mettre le resultat.
	 *		Ce vecteur est remit a zero a chaque appel.
     * @param maxResults Nombre maximum de mots voulus.
     * @param keyWords Chaine de caractere racine de la recherche.
	 * @return Renvoi le nombre de mots trouve.
	 */
	unsigned int FindWordsAtAllCost(std::vector<wstring> & results
		, unsigned int maxResults
		, wstring keyWords);

	/**
	 * @brief Rechercher les mots pouvant completer le texte fournit en entree.
	 * @param results Vecteur de chaines de caracteres dans lequel mettre le resultat.
			Ce vecteur est remit a zero a chaque appel.
	 * @param maxResults Nombre maximum de mots voulus.
	 * @param keyWords Chaine de caractere racine de la recherche.
	 * @param ignoreKeyWords Indique que la recherche ne doit pas se trouver dans results.
	 * @return Renvoi le nombre de mots trouve.
	*/
	unsigned int FindWords(std::vector<wstring> & results
		, unsigned int maxResults
		, wstring keyWords
		, bool ignoreKeyWords = true);

	unsigned int GetPond(wstring word);

	void SetPond(wstring mot, unsigned int pond);

	/**
	 * @brief Incremente le poids du mot fourni en parametre.
	 * @param mot Mot a incrementer.
	 */
	void IncrementerPoids(wstring mot);

	/**
	 * @brief Supprime un mot du dictionnaire.
	 * @param mot Mot a supprimer.
	 */
	void SupprimerMot(wstring mot);

	/**
	 * @brief Supprime tous les mots du dictionnaire.
	 */
	void SupprimerMots();

	/**
	 * @brief Indique si un mot existe dans le dictionnaire.
	 * @param mot Mot a rechercher.
	 * @retval true Le mot existe.
	 * @retval false Le mot n'existe pas.
	 */
	bool ExisteMot(wstring mot);

	/**
	 * @brief Ajoute un mot au dictionnaire.
	 * @param mot Mot a ajouter.
	 */
	void AjouterMot(wstring mot, unsigned int poids = 0);

	/**
	 * @brief Remet a zero le poid du mot dans le dictionnaire.
	 * @param mot Mot pour lequel il faut mettre a zero le poids
	 */
	void ResetPoids(wstring mot);

	/* @brief Remet a zero le poid de chaque mot du dictionnaire. */
	void ResetPoids();

	std::deque<MotPondere> GetAllWords();

	Language GetLang() { return lang; }

	BOOL DeleteDico(wstring dico);
	
/* -------------------------------------------------------------------- */
/*                        PRIVATE                                       */
/* -------------------------------------------------------------------- */
private:
/*------------------------------------------------- Fonctions privées   */

	/**
	 * @brief Reorganise la totalite du dictionnaire en fonction du poids
	 * des mots.
	 */
	void trierDictionnaire();

	/**
	 * @brief Filtre les mots presents dans motsPonderes afin qu'ils repondent
	 * au contraites voulues par l'utilisateur.
	 * @param keyWords Texte recherche dans les mots.
	 * @param motsPonderes Vecteur des mots ponderes a filtrer (Ordre decroissant).
	 * @param results Vecteur dans lequel inserer le resultat.
     * @param maxResults Nombre maximum de mot a inserer (Peut etre decremente).
	 * @param ignoreKeyWords Indique que la recherche ne doit pas se trouver dans results.
	 */
	void filtrerVecteurMots(wstring & keyWords
		, std::deque<MotPondere> & motsPonderes
		, std::vector<wstring> & results
		, unsigned int & maxResults
		, bool ignoreKeyWords);
	

	/**
	 * @brief Fourni des mots de dictionnaire dont le style est le meme
	 * que celui de la recherche. Ex: "Rec" -> "Recherche" et "REC" -> "RECHERCHE"
	 * @param keyWords Texte de recherche.
	 * @param results Vecteurs des mots a adapter.
	 */
	void adapterVecteurMots(wstring & keyWords, std::vector<wstring> & results);

	Language lang;
/* -------------------------------------------------------------------- */
/*                        PROTECTED                                     */
/* -------------------------------------------------------------------- */
protected:
/*------------------------------------------------- Variables protegées */
	DictionnaireNiveau2 dico1; // Structure de recherche pour un caractère
	DictionnaireNiveau1 dico2; // Structure de recherche pour deux caractères
	DictionnaireNiveau0 dico; // Structure de recherche pour trois caractères ou plus
};

