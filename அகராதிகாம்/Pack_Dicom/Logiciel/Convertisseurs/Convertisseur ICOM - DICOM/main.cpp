#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
#include <stdlib.h>
#include <windows.h>
#include <locale> // Gestion de isalnum


using namespace std;

int main (int argc, char * argv [])
{
	// Dictionnaire a convertir
	ifstream infile("Dictionnaire.txt",ios::binary);

	// Dictionnaire resultat
	ofstream outfile("DICOM.txt",ios::binary);
	
	// Definition d'un language afin de determiner les alpha num
	locale loc( "French_France" );
	//cout<<(isalnum('\n',loc)?"yes":"no");

	char c; // Caractere lu
	char buffer[255]; // Mot lu
	int nbLus = 0;
	bool lireMot = true; // Indique que le caractere precedent etait pas alpha
	bool premiereLettre = true; // Indique si premiere caractere du mot
	while(!infile.eof()) // Tan que on est pas a la fin
	{
		// Lecture d'un caractere
		infile.read(&c,1);

		if( isalnum(c,loc)||(c=='\'')||(c=='-') ) // Caractere alphanumerique
		{
			if( true==lireMot ) // S'il faut sauvegarder le mot
			{
				if( true==premiereLettre ) // Si premiere lettre du mot
				{
					nbLus = 0; // Aucune caractere lu
					premiereLettre = false;
				}

				// Creation du buffer d'ecriture
				buffer[nbLus++]=c;
			}
		}
		else
		{
			// Si on vient de finir une lecture
			if( true==lireMot )
			{
				// Calcul de la taille du mot
				buffer[nbLus]='\0';

				// Conversion en minuscule du mot
				CharLower(buffer);

				// Sauvegarde du mot
				outfile << 0 << ":" << buffer << std::endl;

				// Fin de lecture du mot
				lireMot = false;
			}

			if( c=='\r' ) // Retour chariot
			{
				// Lecture du caractere suivant
				infile.read(&c,1);
				if( c=='\n' )
				{
					lireMot = true; // Autorise la sauvegarde du mot qui vient
					premiereLettre = true; // Premiere lettre du mot
				}
			}
		}
	}
	
	infile.close();
	outfile.close();

	//lecture avec les librairie du language C

	cout<<"That's all folks... ;-)"<<endl;
	getch();

	return 0;
}