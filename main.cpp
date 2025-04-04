#include <iostream>
#include <fstream>

using namespace std;

int main() {
	string nomeArquivo;
	
	cout << "Digite o nome do arquivo: ";
	cin >> nomeArquivo;
	while (nomeArquivo != "")
	{
		ifstream entrada("./instancias/" + nomeArquivo);
		if(!entrada.is_open()){
			cout << "Erro ao abrir o arquivo de instancia." << endl;
			return 1;
		}
				
		entrada.close();
		
		cout << "Digite o nome do arquivo: ";
		cin >> nomeArquivo;
	}
}
