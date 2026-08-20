/**
* Copyright (C) 2015 Ugo MAROTTE
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
/*								  Outils.h								*/
/*                         ----------------------						*/
/*					  Toolbox pour la synthèse vocale                   */
/*                                                                      */
/*======================================================================*/

#pragma comment (lib, "comctl32.lib")
/* ------------------------------------------------------------ INCLUDE */
/* ---------------------------------------------------- Include système */
#include <windows.h> // COLORREF etc.
#include <shlobj.h> //Browser de dossier
#include <string>
#include <commctrl.h> // Common controls
#include <fstream> // Gestion des fichiers
#include <vector> // Gestion des vecteurs
#include <deque>
#include <sphelper.h>
#include <locale> // Gestion de isalnum
#include <time.h> // Gestion du temps
#include <algorithm> // Utilisation de sort
#include <iostream>
#include <sstream>
#include <atlbase.h>
#include <atlcom.h>
#include <sapi.h>
#include <atlcomcli.h>
#include <thread>
#include <codecvt>

/* -------------------------------------------------- Include personnel */
#include "Configuration.h"
#include "resource.h"
#include "Clavier.h"
#include "GestionnaireBoutons.h"
#include "Dictionnaire.h"
#include "tinyxml.h"

/* -------------------------------------------------------------------- */
/*								VARIABLES                               */
/* ------------------------------------------------ Constantes diverses */
#define _ATL_APARTMENT_THREADED
#define MAX_WORD_LENGTH 64 // Taille maximale d'un mot dans le dictionnaire

#define PATH_DRIVE 1
#define PATH_DIR 2
#define PATH_FILE 3
#define PATH_EXT 4
#define PATH_DRIVE_DIR 12
#define PATH_DRIVE_DIR_FILE 123
#define PATH_FILE_EXT 34
#define PATH_ALL 1234

/* -------------------------------------------------- Constantes dialog */
#define MAIN_DLG_DEFAULT_WIDTH 160 // Taille minimum en largeur du dialogue princ
#define MAIN_DLG_DEFAULT_HEIGHT 320 // Taille minimum en hauteur du dialogue princ
#define TITLE_BAR_HEIGHT 30 // Taille de la barre de titre d'une application
#define IDT_TIMER_HIDE 1 // ID du timer d'affichage
#define IDT_TIMER_SAY 2

/* ---------------------------------------- Constantes noms de fichiers */
#define DEFAULT_DICTIONARY_FILE L"DICOM_DEFAULT"
#define HOOKDLL_FILE_NAME L"HookDLL.dll"
#define CONFIG_FILE_NAME L"DICOM.cfg"

/* ----------------------------------------------------------- DLL Hook */
typedef void(WINAPI *DLLInitHook)(HWND hDest); // Fonction d'init de HOOK
extern DLLInitHook InitHook;
typedef void(WINAPI *DLLEndHook)(); // Fonction de fin de HOOK
extern DLLEndHook EndHook;
typedef void(WINAPI *DLLEnableEnterHook)(bool enable); // Fonction d'activation de entrée (pour selection via focus)
extern DLLEnableEnterHook EnableEnterHook;
typedef void(WINAPI *DLLEnableCharHook)(bool enable); // Fonction d'activation du hook sur les caractères
extern DLLEnableCharHook EnableCharHook;
typedef void(WINAPI *DLLEnableSelectByFunction)(bool enable);
extern DLLEnableSelectByFunction EnableSelectByFunction;
typedef void(WINAPI *DLLEnableSelectByArrows)(bool enable);
extern DLLEnableSelectByArrows EnableSelectByArrows;
typedef void(WINAPI *DLLEnableOtherOption)(bool enable);
extern DLLEnableOtherOption EnableOtherOption;
extern HINSTANCE hinstDLLHook; // Instance la DLL de HOOK

/* ----------------------------------------------- Variables Graphiques */
extern HINSTANCE hInst; // Instance de notre application
extern HWND mainDlg; // Handle de notre dialogue principal
extern HWND configDlg; // Handle de notre dialogue de config
extern HWND dicoDlg; // Handle de notre dialogue de dico
extern HWND insertDlg; // Handle de notre dialogue d'insertion
extern HWND speechDlg; // Handle de notre dialogue de config de la synthèse vocale
extern HWND hListWord;
extern HWND newDicoDlg;
extern HMENU mainDlgSubMenu; //Handle sur le menu minimise
extern int heightButtonDefault; // Hauteur d'un bouton au chargement
extern GestionnaireBoutons gestionnaireBoutons; // Gestionnaire des boutons de l'IHM

/* ---------------------------------------------------------- UGO MODIF */
extern CComModule _Module;

extern HANDLE mutex; // Handle pour le mutex (sert à vérifier qu'une seule instance de DICOM est lancée)
extern HWND editor; // Handle de l'éditeur
extern TRACKMOUSEEVENT tme; // Structure pour tracer les mouvements de la souris
extern NOTIFYICONDATA ndata; // Structure pour l'icône de la barre des tâches
extern bool active; // variable indiquant si le DICOM est activé/désactivé

/* ------------------------------------------------------- Dictionnaire */
extern wstring the_dico_filename;
extern Dictionnaire dictionnaire; // Dictionnaire des mots
extern wstring currentSearch; // Texte de recherche courant

/* ------------------------------------------------------ Configuration */
extern Configuration config;

/* ---------------------------------------------------- Synthèse vocale */
extern ISpVoice* pVoice;		// Voix servant à la synthèse vocale
extern wchar_t *word;		// Mot à prononcer - TODO: passer le mot en paramètre du thread
extern HANDLE sayingHandle;		// Handle du thread
extern BOOL saying;	
extern wstring temp_voice_str;
extern Language lang;
extern CComPtr<IStream> is;

extern wchar_t *path_to_exe;

extern bool writing;

/* -------------------------------------------------------------------- */
/*								FONCTIONS                               */
/* -------------------------------------------------------- Load/Unload */
HRESULT InitSpeech();
void QuitDicom();
void LoadTaskBarStr(bool switchActive = false);

/* ---------------------------------------------------------- Affichage */
void AdapterDialoguePrincipale(bool afficherDialogue);
void AddTaskBarIcon();
void RemoveTaskBarIcon();

void GetItemText(HWND hList, const int &iSel, TCHAR * text);
LRESULT TableDraw(LPARAM lParam);

/* -------------------------------------------------------- Application */
void RechercherDictionnaire();
void WriteWord(int id);
void SetButtonFocus(unsigned int i);

void FillLVDico(HWND hDlg);

LRESULT CALLBACK DlgLoadProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DlgSaveProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DlgQuitProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* --------------------------------------------------- Speech Synthesis */
void SayWord(const wchar_t *w);
void ThreadSpeak(void * params);
void GetVoices(std::deque<wstring> &dequeVoice);
CComPtr<ISpObjectToken> GetVoiceFromString(wstring voice);


/* ----------------------------------------------------- string/wstring */
wstring GetXMLString(std::string name);

std::deque<wstring> GetDicoFiles();
bool ExistFile(wstring filename);

bool LoadDico(wstring filename, bool newDico = false);
void SaveDico(wstring filename);

void MoveToExeDirectory();

void GetParsedPath(const wchar_t *path, INT part, wchar_t *buf);

static bool debug = false;