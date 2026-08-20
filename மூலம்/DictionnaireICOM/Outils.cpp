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
/*								 Outils.cpp								*/
/*                         ----------------------                       */
/*                                                                      */
/*======================================================================*/

/* ------------------------------------------------------------ INCLUDE */
/* -------------------------------------------------- Include personnel */
#include "Outils.h"

/* -------------------------------------------------------------------- */
/*								VARIABLES                               */
/* ----------------------------------------------------------- DLL Hook */
DLLInitHook InitHook = NULL;
DLLEndHook EndHook = NULL;
DLLEnableEnterHook EnableEnterHook = NULL;
DLLEnableCharHook EnableCharHook = NULL;
DLLEnableSelectByFunction EnableSelectByFunction = NULL;
DLLEnableSelectByArrows EnableSelectByArrows = NULL;
DLLEnableOtherOption EnableOtherOption = NULL;
HINSTANCE hinstDLLHook; // Instance la DLL de HOOK

/* ----------------------------------------------- Variables Graphiques */
HINSTANCE hInst; // Instance de notre application
HWND mainDlg = NULL; // Handle de notre dialogue principal
HWND configDlg = NULL; // Handle de notre dialogue de config
HWND dicoDlg = NULL; // Handle de notre dialogue de dico
HWND insertDlg = NULL; // Handle de notre dialogue d'insertion
HWND speechDlg = NULL; // Handle de notre dialogue de config de la synthèse vocale
HWND hListWord = NULL;
HWND newDicoDlg = NULL;
HMENU mainDlgSubMenu = NULL; //Handle sur le menu minimise
int heightButtonDefault; // Hauteur d'un bouton au chargement
GestionnaireBoutons gestionnaireBoutons; // Gestionnaire des boutons de l'IHM

/* ---------------------------------------------------------- UGO MODIF */
extern CComModule _Module;

HANDLE mutex; // Handle pour le mutex (sert à vérifier qu'une seule instance de DICOM est lancée)
HWND editor = NULL; // Handle de l'éditeur
TRACKMOUSEEVENT tme; // Structure pour tracer les mouvements de la souris
NOTIFYICONDATA ndata = {}; // Structure pour l'icône de la barre des tâches
bool active = true; // variable indiquant si le DICOM est activé/désactivé
wchar_t *word;
ISpVoice* pVoice = NULL;
HANDLE sayingHandle;
BOOL saying;
wstring temp_voice_str;
CComPtr<IStream> is;
Language lang;
wchar_t *path_to_exe = NULL;

/* ------------------------------------------------------- Dictionnaire */
wstring the_dico_filename;
Dictionnaire dictionnaire; // Dictionnaire des mots
wstring currentSearch; // Texte de recherche courant

/* ------------------------------------------------------ Configuration */
Configuration config;

bool writing = false;

/* -------------------------------------------------------------------- */
/*								FONCTIONS                               */
/* ---------------------------------------------------------Load/Unload */
HRESULT InitSpeech() {
	HRESULT result = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(result)) {
		result = CoCreateInstance(CLSID_SpVoice,
			NULL,
			CLSCTX_ALL,
			IID_ISpVoice,
			(void **)&pVoice);
	}
	if (SUCCEEDED(result)) {
		saying = false;
		wstring strVoice = config.GetStringParam(VOICE_SELECTED_VOICE);
		wchar_t wVoice[256];
		wcscpy(wVoice, strVoice.c_str());
		CComPtr<ISpObjectToken> token = GetVoiceFromString(wVoice);
		if (token != NULL) {
			pVoice->SetVoice(token);
		}
		pVoice->SetVolume((USHORT)config.GetUnsignedIntParam(VOICE_VOLUME));
		pVoice->SetRate(config.GetRate());

	}
	if (FAILED(result)) {
		char error[256];
		sprintf_s(error, "%s%s%s\n%s",
			GetXMLString(STR_ERROR_SPEECH_1),
			GetXMLString(STR_ERROR_SPEECH_2),
			GetXMLString(STR_ERROR_SPEECH_3),
			GetXMLString(STR_ERROR_SPEECH_4));
		MessageBox(NULL
			, Utf8ToUtf16(error).c_str()
			, GetXMLString(STR_ERROR_SPEECH_TITLE).c_str()
			, MB_ICONERROR | MB_OK | MB_TASKMODAL);
	}
	return result;
}

void QuitDicom() {
	if (debug) OutputDebugStringA("	ID_POPUP_QUITTER\n"); 
	// Recuperation des informations d'affichage de la fenetre
	ShowWindow(mainDlg, SW_HIDE);
	gestionnaireBoutons.SetNbBoutons(NB_MAX_BUTTONS);
	active = true;
	AdapterDialoguePrincipale(active);
	// Recuperation de la taille du dialogue courant
	RECT dlgRect;
	GetWindowRect(mainDlg, &dlgRect);
	// Sauvegarde des informations dans la configuration
	config.SetStringParam(FONT_TYPE, gestionnaireBoutons.GetPolice());
	config.SetUnsignedIntParam(FONT_TEXT_COLOR, gestionnaireBoutons.GetCouleurText());
	config.SetUnsignedIntParam(FONT_TEXT_FOCUS_COLOR, gestionnaireBoutons.GetCouleurFocusText());
	config.SetUnsignedIntParam(DIALOG_BACKGROUND_COLOR, gestionnaireBoutons.GetCouleurBk());
	config.SetUnsignedIntParam(DIALOG_BACKGROUND_FOCUS_COLOR, gestionnaireBoutons.GetCouleurFocusBk());
	config.SetBoolParam(FONT_BOLD, gestionnaireBoutons.GetPoliceBold());
	config.SetBoolParam(FONT_ITALIC, gestionnaireBoutons.GetPoliceItalic());
	// Sauvegarde des dimensions du dialogue principal
	config.SetUnsignedIntParam(DIALOG_POS_X, dlgRect.left);
	config.SetUnsignedIntParam(DIALOG_POS_Y, dlgRect.top);
	config.SetUnsignedIntParam(DIALOG_WIDTH, dlgRect.right - dlgRect.left);
	config.SetUnsignedIntParam(DIALOG_HEIGHT, dlgRect.bottom - dlgRect.top);
	// Suppression de l'icone de la barre des taches
	RemoveTaskBarIcon();
	::CoUninitialize();
	KillTimer(mainDlg, IDT_TIMER_HIDE);
	EndDialog(mainDlg, 0); 
	PostQuitMessage(0); // Fin de l'application
}

/* ---------------------------------------------------------- Affichage */
/*
* @brief Cette fonction a pour role de definir la taille du dialogue
* principale en fonction du nombre de boutons a afficher et la taille
* courante des boutons.
* @param afficherDialogue Indique si l'affichage du dialogue doit avoir
* lieu apres adaptation. TRUE le dialogue sera affiche.
*/
void AdapterDialoguePrincipale(bool afficherDialogue = true)
{
	wchar_t str_debug[256];
	wsprintf(str_debug, L"Adaptation du dialogue principal... %s %i\n", currentSearch.c_str(), gestionnaireBoutons.GetNbBoutons());
	if (debug) OutputDebugString(str_debug);
	// Indique si cette adaptation est la premire (coin haut droite)
	static bool premiereAdaptation = true;
	if (true == premiereAdaptation) // Tous les boutons doivent etre initialise
		gestionnaireBoutons.SetNbBoutons(NB_MAX_BUTTONS);

	// Taille du premier bouton de l'interface
	RECT ctrlRect; GetClientRect(GetDlgItem(mainDlg, IDC_BUTTON_F1), &ctrlRect);

	// Affichage des boutons dont l'index est plus petit que GetNbBoutons()
	for (unsigned int i = 0; i<NB_MAX_BUTTONS; ++i) {
		HWND hButton = GetDlgItem(mainDlg, i + IDC_BUTTON_F1);
		if (i < gestionnaireBoutons.GetNbBoutons()) { // Affichage du bouton
			SendMessage(hButton, WM_SETTEXT, 0, (WPARAM)NULL);
			ShowWindow(hButton, SW_SHOW);
		}
		else // Hide du bouton
			ShowWindow(hButton, SW_HIDE);
	}

	// Lecture des coordonnees du dialogue
	RECT dlgRect;
	GetWindowRect(mainDlg, &dlgRect);

	// Lecture des dimension de la zone client
	RECT dlgClientRect;
	GetClientRect(mainDlg, &dlgClientRect);

	POINT p;
	GetCursorPos(&p);

	// Dimensionnement de la fenetre pour qu'elle accepte tous les boutons
	if (false == premiereAdaptation) {

		int x = p.x;
		int y = p.y + 15;
		int w = dlgRect.right - dlgRect.left;
		int h = ((dlgRect.bottom - dlgRect.top) - dlgClientRect.bottom)
			+ 20
			+ (gestionnaireBoutons.GetNbBoutons() == 0 ? 1 : gestionnaireBoutons.GetNbBoutons())*ctrlRect.bottom;

		MoveWindow(mainDlg
			, x
			, y
			, w
			, h
			, TRUE);
	}
	else {
		premiereAdaptation = false;

		gestionnaireBoutons.SetNbBoutons(0); // Aucun boutons
	}

	// Réadaptation dans le cas ou la fenêtre sort de l'écran
	RECT desk;
	GetWindowRect(GetDesktopWindow(), &desk);
	GetWindowRect(mainDlg, &dlgRect);
	GetClientRect(mainDlg, &dlgClientRect);

	int top = dlgRect.top, bot = dlgRect.bottom, right = dlgRect.right, left = dlgRect.left;

	int x = left;
	int y = top;
	int w = right - left;
	int h = bot - top;


	if (dlgRect.right > desk.right)
		x = p.x - w;

	if (dlgRect.bottom > desk.bottom)
		y = p.y - h - 15;

	MoveWindow(mainDlg
		, x
		, y
		, w
		, h
		, TRUE);

	GetClientRect(mainDlg, &dlgClientRect);
	InvalidateRect(mainDlg, &dlgClientRect, false);

	// Affichage de la fenetre si nbButtons > 0
	if (gestionnaireBoutons.GetNbBoutons() > 0 && active) {
		// Affichage de la fenetre principale sans perdre la fenetre en foreground
		HWND currentHWND = GetForegroundWindow();
		if (afficherDialogue) ShowWindow(mainDlg, SW_SHOW);
		SetForegroundWindow(currentHWND);

		// Mise en place du timer de reduction
		SetTimer(mainDlg, IDT_TIMER_HIDE, config.GetUnsignedIntParam(SPLASH_TIME), (TIMERPROC)NULL);
	}
	else // Sinon on cache la fenetre principale
		ShowWindow(mainDlg, SW_HIDE);

	gestionnaireBoutons.PositionnerBoutons();
}

/*
* @brief Insere une icone d'application dans la barre des taches.
*/
void AddTaskBarIcon()
{
	if (debug) OutputDebugStringA("Adding taskbar icon...\n");
	// Structure d'insertion dans la barre des taches
	ndata.cbSize = sizeof(NOTIFYICONDATA);
	ndata.hWnd = mainDlg;
	ndata.uID = ID_TASK_BAR;//Id unique de l'instance dans la task barre
	ndata.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;//Indique les champs valide
	ndata.uCallbackMessage = WM_COMMAND; // message qui sera envoyé à la fenêtre.
	ndata.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON_APPLICATION)); // icone qui sera affiché dans la barre des taches

	// Definition de l'infobulle
	wcscpy(ndata.szTip, L"DICOM");

	// Ajout
	Shell_NotifyIcon(NIM_ADD, &ndata);
	LoadTaskBarStr();
}

/*
* @brief Supprime une icone d'application de la barre des taches.
*/
void RemoveTaskBarIcon()
{
	if (debug) OutputDebugStringA("Removing taskbar icon...\n");
	// Suppression
	Shell_NotifyIcon(NIM_DELETE, &ndata);
}

void LoadTaskBarStr(bool switchActive) {
	MENUITEMINFOW mii = {};
	mii.cbSize = sizeof(MENUITEMINFOW);
	mii.fMask = MIIM_STRING;
	wchar_t text[256];
	wsprintf(text, active ? GetXMLString(STR_CTX_DEACTIVATE).c_str() : GetXMLString(STR_CTX_ACTIVATE).c_str());
	mii.dwTypeData = text;
	SetMenuItemInfo(mainDlgSubMenu, ID_POPUP_DISABLE, false, &mii);

	if (!switchActive) {
		wsprintf(text, GetXMLString(STR_CTX_CONFIG).c_str());
		mii.dwTypeData = text;
		SetMenuItemInfo(mainDlgSubMenu, ID_POPUP_CONFIGURER, false, &mii);

		wsprintf(text, GetXMLString(STR_CTX_DICO).c_str());
		mii.dwTypeData = text;
		SetMenuItemInfo(mainDlgSubMenu, ID_POPUP_DICTIONNAIRE, false, &mii);

		wsprintf(text, GetXMLString(STR_CTX_QUIT).c_str());
		mii.dwTypeData = text;
		SetMenuItemInfo(mainDlgSubMenu, ID_POPUP_QUITTER, false, &mii);
	}
}

/* -------------------------------------------------------- Application */
/*
* @brief Cette fonction a pour role de faire la recherche dans le dictionnaire de mot
* de la variable globale "currentSearch". Le resultat de la recherche est affecte
* au differents boutons en fonction de leur pertinence.
*/
void RechercherDictionnaire()
{
	if (debug) OutputDebugStringA("Searching dictionnary...\n");
	// Effectue une recherche des que l'on depasse le nombre de lettres minimum
	if (config.GetUnsignedIntParam(SPLASH_NB_LETTERS) <= currentSearch.length()) {
		// Recherche des mots disponibles et sauvegarde du nombre de resultats
		std::vector<wstring> findWords;
		unsigned int nbw = dictionnaire.FindWordsAtAllCost(findWords
			, NB_MAX_BUTTONS
			, currentSearch);
		gestionnaireBoutons.SetNbBoutons(nbw);
		// Affectation des resultats pour chaque boutons de l'interface
		for (unsigned int i = 0; i<gestionnaireBoutons.GetNbBoutons(); ++i)
			gestionnaireBoutons.SetBoutonText(i, findWords[i].c_str());
	}
	else {
		gestionnaireBoutons.SetNbBoutons(0);
	}

	// Raffraichissement final
	SetButtonFocus(-1);
}

void WriteWord(int id) {
	if (debug) OutputDebugStringA("Writing word...\n");
	if (NULL != editor && GetForegroundWindow() != editor)
		SetForegroundWindow(editor);

	wstring da_word = gestionnaireBoutons.GetBoutonText(id, L"");

	dictionnaire.IncrementerPoids(da_word);
	unsigned int minIndex = currentSearch.length();
	for (unsigned int i = 0; i < currentSearch.length(); i++) {
		if (da_word[i] != currentSearch[i]) {
			minIndex = i;
			break;
		}
	}

	if (minIndex < currentSearch.length()) {
		wstring temp_cur_search = currentSearch;
		while (temp_cur_search.length() > minIndex) {
			Clavier::SimulerBackSpace();
			temp_cur_search.resize(temp_cur_search.length() - 1);
		}
		currentSearch = temp_cur_search;
	}

	std::wstring txt = gestionnaireBoutons.GetBoutonText(id,currentSearch);
	EnableCharHook(false);
	EnableOtherOption(true);
	Clavier::SimulerTexte(txt.c_str());
	if (config.GetBoolParam(VOICE_ACTIVATION_INSERT))
		SayWord(gestionnaireBoutons.GetBoutonText(id, L"").c_str());
	currentSearch.clear();
	Clavier::SimulerTouche(0x20);
}

void SetButtonFocus(unsigned int i) {
	if (debug) {
		char str_debug[256];
		sprintf_s(str_debug, "Setting focus... %i\n", i);
		OutputDebugStringA(str_debug);
	}
	gestionnaireBoutons.SetBoutonFocus(i);

	if (i != -1 && active) {
		EnableEnterHook(false);
		SetTimer(mainDlg, IDT_TIMER_SAY, 1000, (TIMERPROC)NULL);
	}
	else {
		KillTimer(mainDlg, IDT_TIMER_SAY);
		EnableEnterHook(true);
	}
	AdapterDialoguePrincipale(active);
}

void FillLVDico(HWND hDlg) {
	wchar_t buf[MAX_WORD_LENGTH];

	GetDlgItemText(hDlg, IDC_EDIT_MOT, buf, sizeof(buf));

	// Recherche dans le dico
	std::vector<wstring> findWords;
	dictionnaire.FindWords(findWords
		, 50
		, buf
		, false);
	// Suppression des mots deja present
	SendMessage(hListWord, LVM_DELETEALLITEMS, 0, 0);
	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.mask = LVIF_TEXT;
	lvi.cchTextMax = 256;
	int iItem = 0;
	//SendMessage(hDlg, )
	// Ajout des mots dans la liste
	for (std::vector<wstring>::iterator iter = findWords.begin();
		iter != findWords.end();
		++iter) {
		lvi.iItem = iItem;
		lvi.iSubItem = 0;
		wchar_t buf[256];
		wcscpy(buf, iter->c_str());
		lvi.pszText = &buf[0];
		SendMessage(hListWord, LVM_SETITEMSTATE, iItem, (LPARAM)&lvi);
		SendMessage(hListWord, LVM_INSERTITEM, 0, (LPARAM)&lvi);

		lvi.iSubItem = 1;
		wchar_t poids[11];
		_itow(dictionnaire.GetPond(*iter), poids, 10);
		lvi.pszText = &poids[0];

		SendMessage(hListWord, LVM_SETITEM, 0, (LPARAM)&lvi);

		iItem++;
	}
}

/* --------------------------------------------------- Speech Synthesis */
void SayWord(const wchar_t *w) {
	if (!saying) {
		const wchar_t * cw = w;
		is.Release();
		::CreateStreamOnHGlobal(0, TRUE, &is);
		while (*cw) {
			BYTE b = (BYTE)*cw;
			is->Write(&b, sizeof(b), NULL);
			cw++;
		}
		sayingHandle = (HANDLE)_beginthread(ThreadSpeak, 0, NULL);
	}
}

void ThreadSpeak(void * params) {
	saying = true;
	pVoice->SpeakStream(is, SPF_DEFAULT, NULL);
	saying = false;
	_endthreadex(1);
}

void GetVoices(std::deque<wstring> &dequeVoice) {
	HRESULT hr = S_OK;
	CComPtr<IEnumSpObjectTokens> cpEnum;
	CComPtr<ISpObjectToken> token;
	ULONG count;
	
	wstring regedit = SPCAT_VOICES;
	hr = SpEnumTokens(regedit.c_str(), NULL, NULL, &cpEnum);
	if (SUCCEEDED(hr))
	{
		hr = cpEnum->GetCount(&count);
	}
	while (SUCCEEDED(hr) && count--) {
		token.Release();
		if (SUCCEEDED(hr)) {
			hr = cpEnum->Next(1, &token, NULL);
		}
		if (SUCCEEDED(hr)) {
			wchar_t * ws;
			hr = token->GetStringValue(NULL, &ws);
			if (SUCCEEDED(hr)) {
				dequeVoice.push_back(ws);
			}
		}
	}
}

CComPtr<ISpObjectToken> GetVoiceFromString(wstring voice) {
	HRESULT hr = S_OK;
	CComPtr<IEnumSpObjectTokens> cpEnum;
	CComPtr<ISpObjectToken> token = NULL;
	ULONG count;

	hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
	if (SUCCEEDED(hr))
	{
		hr = cpEnum->GetCount(&count);
	}
	while (SUCCEEDED(hr) && count--) {
		token.Release();
		hr = cpEnum->Next(1, &token, NULL);
		
		if (SUCCEEDED(hr)) {
			wchar_t *ws;
			hr = token->GetStringValue(NULL, &ws);
			if (SUCCEEDED(hr)) {
				if (voice.compare(ws) == 0) {
					return token;
				}
			}
		}
	}
	return NULL;
}



wstring GetXMLString(std::string name) {
	char * filename;
	switch (lang){
	case FR: filename = "XMLSTRFR.xml"; break;
	case EN: filename = "XMLSTREN.xml"; break;
	default: return L"NULL";
	}
	TiXmlDocument doc(filename);
	if (doc.LoadFile()) {
		TiXmlElement* root = doc.RootElement();
		if (root) {
			TiXmlElement* elem = root->FirstChildElement();
			while (elem) {
				if (strcmp(elem->Attribute("name"), name.c_str()) == 0) {
					return Utf8ToUtf16(elem->GetText()).c_str();
				}

				elem = elem->NextSiblingElement();
			}
		}
	}
	return L"NULL";
}

void GetItemText(HWND hList, const int &iSel, TCHAR * text)
{
	TCHAR item[MAX_PATH] = { 0 };
	LVITEM listItem;
	listItem.mask = LVIF_TEXT;
	listItem.iItem = iSel;
	listItem.pszText = item;
	listItem.cchTextMax = MAX_PATH;
	for (int i = 0; i <= 6; i++)
	{
		listItem.iSubItem = i;
		SendMessage(hList, LVM_GETITEMTEXT, iSel, (LPARAM)&listItem);
		wcscat(text, item);
	}
}

LRESULT TableDraw(LPARAM lParam)
{
	int iRow;
	LPNMLVCUSTOMDRAW pListDraw = (LPNMLVCUSTOMDRAW)lParam;
	switch (pListDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW);
	case CDDS_ITEMPREPAINT:
		iRow = (int)pListDraw->nmcd.dwItemSpec;
		if (iRow % 2 == 0)
		{
			//pListDraw->clrText   = RGB(252, 177, 0);
			pListDraw->clrTextBk = RGB(202, 221, 250);
			return CDRF_NEWFONT;
		}
		else {
			//pListDraw->clrText = RGB(255, 255, 255);
			pListDraw->clrTextBk = RGB(175, 221, 250);

			return CDRF_NEWFONT;
		}
	case CDDS_SUBITEM:
		return CDRF_NEWFONT;
	default:
		break;
	}
	return CDRF_DODEFAULT;
}

std::deque<wstring> GetDicoFiles() {
	WIN32_FIND_DATA ffd;
	wchar_t szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	std::deque<wstring> dicoFiles;

	wsprintf(szDir, _T(".\\Dictionnaires\\*.dic"));
	hFind = FindFirstFile(szDir, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return dicoFiles;
	}

	PathRemoveExtension(ffd.cFileName);
	if (StrCmp(ffd.cFileName, DEFAULT_DICTIONARY_FILE) != 0)
		dicoFiles.push_back(ffd.cFileName);

	while (FindNextFile(hFind, &ffd) != 0) {
		PathRemoveExtension(ffd.cFileName);
		if (StrCmp(ffd.cFileName, DEFAULT_DICTIONARY_FILE) != 0)
			dicoFiles.push_back(PathFindFileName(ffd.cFileName));
	}

	return dicoFiles;
}

bool ExistFile(wstring filename) {
	std::deque<wstring> dicoFiles = GetDicoFiles();

	for (std::deque<wstring>::iterator iter = dicoFiles.begin();
		iter != dicoFiles.end();
		++iter) {
		if (iter->compare(filename) == 0)
			return TRUE;
	}

	return FALSE;
}

LRESULT CALLBACK DlgLoadProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_STATIC_LOADING, GetXMLString(STR_LOADING_DICOM).c_str());
		break;
	default: break;
	}
	return FALSE;
}

LRESULT CALLBACK DlgSaveProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_STATIC_LOADING, GetXMLString(STR_SAVING_DICOM).c_str());
		break;
	default: break;
	}
	return FALSE;
}

LRESULT CALLBACK DlgQuitProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, IDC_STATIC_LOADING, GetXMLString(STR_CLOSING_DICOM).c_str());
		break;
	default: break;
	}
	return FALSE;
}

bool LoadDico(wstring filename, bool newDico) {
	MoveToExeDirectory();
	clock_t tempsAvant = clock();
	HWND cacheDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_DEMARRAGE), mainDlg, (DLGPROC)DlgLoadProc);
	bool loaded = dictionnaire.LoadFromFile(filename, newDico);
	clock_t tempsAttente = 1000 - (clock() - tempsAvant);
	if (tempsAttente > 0) Sleep(tempsAttente); // Affichage mini de 1s
	DestroyWindow(cacheDialog);

	if (false == loaded) {
		wchar_t error[2048];
		wsprintf(error, L"%s%s\n%s",
			GetXMLString(STR_ERROR_DICO_1).c_str(),
			GetXMLString(STR_ERROR_DICO_2).c_str(),
			GetXMLString(STR_ERROR_DICO_3).c_str());
		MessageBox(NULL
			, error
			, GetXMLString(STR_ERROR_DICO_TITLE).c_str()
			, MB_ICONERROR | MB_OK | MB_TASKMODAL);
	}
	return loaded;
}

void SaveDico(wstring filename) {
	MoveToExeDirectory();
	if (the_dico_filename.length() == 0)
		return;
	clock_t tempsAvant = clock();
	HWND cacheDialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_DEMARRAGE), mainDlg, (DLGPROC)DlgSaveProc);
	bool saved = dictionnaire.SaveIntoFile(filename);
	clock_t tempsAttente = 1000 - (clock() - tempsAvant);
	if (tempsAttente > 0) Sleep(tempsAttente); // Affichage mini de 1s
	DestroyWindow(cacheDialog);
}

void MoveToExeDirectory() {
	wchar_t dir[2048];
	if (path_to_exe == NULL) {
		wchar_t buf[256];
		GetModuleFileName(NULL, buf, sizeof(buf));
		path_to_exe = new wchar_t[256];
		GetParsedPath(buf, PATH_DRIVE_DIR, path_to_exe);
	}
	SetCurrentDirectory(path_to_exe);
}

void GetParsedPath(const wchar_t * path, INT part, wchar_t *buf) {
	wchar_t drive[256], dir[256], filename[256], ext[256];
	_wsplitpath(path, drive, dir, filename, ext);
	switch (part) {
	case PATH_DRIVE: wsprintf(buf, L"%s\0", drive); break;
	case PATH_DIR: wsprintf(buf, L"%s\0", dir); break;
	case PATH_FILE: wsprintf(buf, L"%s\0", filename); break;
	case PATH_EXT: wsprintf(buf, L"%s\0", ext); break;
	case PATH_DRIVE_DIR: wsprintf(buf, L"%s%s\0", drive, dir); break;
	case PATH_DRIVE_DIR_FILE: wsprintf(buf, L"%s%s%s\0", drive, dir, filename); break;
	case PATH_FILE_EXT: wsprintf(buf, L"%s%s\0", filename, ext); break;
	case PATH_ALL: wsprintf(buf, L"%s%s%s%s\0", drive, dir, filename, ext); break;
	default: break;
	}
}

