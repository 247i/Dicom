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
	ifstream infile("Dictionnaire.txt");

	// Dictionnaire resultat
	ofstream outfile("DICOM.txt");

	std::string line;
	// Tan qu'il ya des lignes a lire
	while(std::getline(infile, line))
	{
		line.insert(0, "0:");
		line.append("\n");
		outfile.write(line.c_str(), line.length());
	}
	
	infile.close();
	outfile.close();

	//lecture avec les librairie du language C

	cout<<"That's all folks... ;-)"<<endl;
	getch();

	return 0;
}