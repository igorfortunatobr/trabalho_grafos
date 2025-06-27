#include "grafo.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept> // Para std::runtime_error

using namespace std;

// Função auxiliar para extrair valor inteiro de uma linha "CHAVE : VALOR"
int iExtrairValorInteiro(const string& sLinha, const string& sChave) {
    size_t iPos = sLinha.find(":");
    if (iPos == string::npos) {
        cerr << "DEBUG: Formato invalido (sem ':') na linha para chave '" << sChave << "': " << sLinha << endl;
        return 0; // Ou lançar exceção
    }
    string sValorStr = sLinha.substr(iPos + 1);
    // Remover espaços em branco antes do número
    sValorStr.erase(0, sValorStr.find_first_not_of(" \t"));
    try {
        return stoi(sValorStr);
    } catch (const std::invalid_argument& ia) {
        cerr << "DEBUG: Erro ao converter para inteiro (invalid_argument) na linha para chave '" << sChave << "': " << sLinha << " | Valor string: '" << sValorStr << "'" << endl;
        return 0; // Ou lançar exceção
    } catch (const std::out_of_range& oor) {
        cerr << "DEBUG: Erro ao converter para inteiro (out_of_range) na linha para chave '" << sChave << "': " << sLinha << " | Valor string: '" << sValorStr << "'" << endl;
        return 0; // Ou lançar exceção
    }
}

sGrafo sLerArquivo(const string& sNomeArquivo, bool& bConseguiuLerArquivo) {
    bConseguiuLerArquivo = false; // Inicializa como falso
    sGrafo sGrafoLido;
    cout << "DEBUG: Tentando abrir arquivo: " << sNomeArquivo << endl;
    ifstream arquivo("./instancias/" + sNomeArquivo);

    if (!arquivo.is_open()) {
        cerr << "DEBUG: Falha ao abrir o arquivo: " << sNomeArquivo << endl;
        return sGrafoLido; // Retorna grafo vazio, bConseguiuLerArquivo continua false
    }
    cout << "DEBUG: Arquivo aberto com sucesso: " << sNomeArquivo << endl;

    string sLinha;
    try {
        // Leitura do cabeçalho
        cout << "DEBUG: Lendo cabecalho..." << endl;
        getline(arquivo, sLinha); sGrafoLido.nome = sLinha.substr(sLinha.find(":") + 1);
        sGrafoLido.nome = sGrafoLido.nome.substr(sGrafoLido.nome.find_first_not_of(" \t")); // Remover espaços iniciais
        cout << "DEBUG: Nome: " << sGrafoLido.nome << endl;

        getline(arquivo, sLinha); sGrafoLido.valorOtimo = iExtrairValorInteiro(sLinha, "VALOR OTIMO");
        getline(arquivo, sLinha); sGrafoLido.numVeiculos = iExtrairValorInteiro(sLinha, "NUMERO DE VEICULOS");
        getline(arquivo, sLinha); sGrafoLido.capacidadeVeiculo = iExtrairValorInteiro(sLinha, "CAPACIDADE DOS VEICULOS");
        getline(arquivo, sLinha); sGrafoLido.deposito = iExtrairValorInteiro(sLinha, "DEPOSITO");
        getline(arquivo, sLinha); sGrafoLido.iNumVertices = iExtrairValorInteiro(sLinha, "NUMERO DE VERTICES");
        getline(arquivo, sLinha); sGrafoLido.numArestas = iExtrairValorInteiro(sLinha, "NUMERO DE ARESTAS");
        getline(arquivo, sLinha); sGrafoLido.numArcos = iExtrairValorInteiro(sLinha, "NUMERO DE ARCOS");
        getline(arquivo, sLinha); sGrafoLido.numVerticesRequeridos = iExtrairValorInteiro(sLinha, "NUMERO DE VERTICES REQUERIDOS");
        getline(arquivo, sLinha); sGrafoLido.numArestasRequeridas = iExtrairValorInteiro(sLinha, "NUMERO DE ARESTAS REQUERIDAS");
        getline(arquivo, sLinha); sGrafoLido.numArcosRequeridos = iExtrairValorInteiro(sLinha, "NUMERO DE ARCOS REQUERIDOS");
        cout << "DEBUG: Cabecalho lido. Vertices: " << sGrafoLido.iNumVertices << " DEPOSITO: " << sGrafoLido.deposito << endl;

        // Pular linhas de seção
        getline(arquivo, sLinha); // VERTICES REQUERIDOS
        cout << "DEBUG: Secao: " << sLinha << endl;
        getline(arquivo, sLinha); // (id demanda custo)
        cout << "DEBUG: Cabecalho secao: " << sLinha << endl;

        // Ler vértices requeridos
        cout << "DEBUG: Lendo vertices requeridos..." << endl;
        while (getline(arquivo, sLinha) && !sLinha.empty() && sLinha.find("ARESTAS REQUERIDAS") == string::npos) {
            stringstream ss(sLinha);
            string sIdTexto; int iDemanda, iCusto;
            ss >> sIdTexto >> iDemanda >> iCusto;
            if (sIdTexto.empty() || sIdTexto[0] != 'N') continue; // Ignora linhas vazias ou mal formatadas
            int iId = stoi(sIdTexto.substr(1));
            sGrafoLido.vsVertices.push_back({iId, true, iDemanda, iCusto});
            cout << "DEBUG: Vertice Requerido Lido: ID=" << iId << ", Dem=" << iDemanda << ", Custo=" << iCusto << endl;
        }
        cout << "DEBUG: Terminou de ler vertices requeridos. Linha atual: " << sLinha << endl;

        // Ler arestas requeridas
        cout << "DEBUG: Lendo arestas requeridas..." << endl;
        getline(arquivo, sLinha); // (id orig dest custo_transito demanda custo_atendimento)
        cout << "DEBUG: Cabecalho secao: " << sLinha << endl;
        while (getline(arquivo, sLinha) && !sLinha.empty() && sLinha.find("ARESTAS NAO REQUERIDAS") == string::npos) {
            stringstream ss(sLinha);
            string sIdTexto;
            int iOrigem, iDestino, iCustoTransito, iDemanda, iCustoAtendimento;
            ss >> sIdTexto >> iOrigem >> iDestino >> iCustoTransito >> iDemanda >> iCustoAtendimento;
            if (sIdTexto.empty() || sIdTexto[0] != 'E') continue;
            sGrafoLido.vsArestas.push_back({stoi(sIdTexto.substr(1)), iOrigem, iDestino, iCustoTransito, iDemanda, iCustoAtendimento, true});
            cout << "DEBUG: Aresta Requerida Lida: ID=" << sIdTexto.substr(1) << endl;
        }
        cout << "DEBUG: Terminou de ler arestas requeridas. Linha atual: " << sLinha << endl;

        // Ler arestas não requeridas
        cout << "DEBUG: Lendo arestas nao requeridas..." << endl;
        getline(arquivo, sLinha); // (id orig dest custo_transito)
        cout << "DEBUG: Cabecalho secao: " << sLinha << endl;
        while (getline(arquivo, sLinha) && !sLinha.empty() && sLinha.find("ARCOS REQUERIDOS") == string::npos) {
            stringstream ss(sLinha);
            string sIdTexto;
            int iOrigem, iDestino, iCustoTransito;
            ss >> sIdTexto >> iOrigem >> iDestino >> iCustoTransito;
            if (sIdTexto.empty() || sIdTexto.substr(0, 3) != "NrE") continue;
            sGrafoLido.vsArestas.push_back({stoi(sIdTexto.substr(3)), iOrigem, iDestino, iCustoTransito, 0, 0, false});
            cout << "DEBUG: Aresta Nao Requerida Lida: ID=" << sIdTexto.substr(3) << endl;
        }
        cout << "DEBUG: Terminou de ler arestas nao requeridas. Linha atual: " << sLinha << endl;

        // Ler arcos requeridos
        cout << "DEBUG: Lendo arcos requeridos..." << endl;
        getline(arquivo, sLinha); // (id orig dest custo_transito demanda custo_atendimento)
        cout << "DEBUG: Cabecalho secao: " << sLinha << endl;
        while (getline(arquivo, sLinha) && !sLinha.empty() && sLinha.find("ARCOS NAO REQUERIDOS") == string::npos) {
            stringstream ss(sLinha);
            string sIdTexto;
            int iOrigem, iDestino, iCustoTransito, iDemanda, iCustoAtendimento;
            ss >> sIdTexto >> iOrigem >> iDestino >> iCustoTransito >> iDemanda >> iCustoAtendimento;
            if (sIdTexto.empty() || sIdTexto[0] != 'A') continue;
            sGrafoLido.vsArcos.push_back({stoi(sIdTexto.substr(1)), iOrigem, iDestino, iCustoTransito, iDemanda, iCustoAtendimento, true});
            cout << "DEBUG: Arco Requerido Lido: ID=" << sIdTexto.substr(1) << endl;
        }
        cout << "DEBUG: Terminou de ler arcos requeridos. Linha atual: " << sLinha << endl;

        // Ler arcos não requeridos
        cout << "DEBUG: Lendo arcos nao requeridos..." << endl;
        getline(arquivo, sLinha); // (id orig dest custo_transito)
        cout << "DEBUG: Cabecalho secao: " << sLinha << endl;
        while (getline(arquivo, sLinha) && !sLinha.empty() && sLinha.find("FIM") == string::npos) { // Ler até FIM
            stringstream ss(sLinha);
            string sIdTexto;
            int iOrigem, iDestino, iCustoTransito;
            ss >> sIdTexto >> iOrigem >> iDestino >> iCustoTransito;
            if (sIdTexto.empty() || sIdTexto.substr(0, 3) != "NrA") continue;
            sGrafoLido.vsArcos.push_back({stoi(sIdTexto.substr(3)), iOrigem, iDestino, iCustoTransito, 0, 0, false});
            cout << "DEBUG: Arco Nao Requerido Lido: ID=" << sIdTexto.substr(3) << endl;
        }
        cout << "DEBUG: Terminou de ler arcos nao requeridos. Linha atual: " << sLinha << endl;

        // Se chegou até aqui sem exceções, a leitura foi bem-sucedida
        bConseguiuLerArquivo = true;
        cout << "DEBUG: Leitura concluida com sucesso para " << sNomeArquivo << endl;

    } catch (const std::exception& e) {
        cerr << "DEBUG: Excecao durante a leitura do arquivo " << sNomeArquivo << ": " << e.what() << endl;
        bConseguiuLerArquivo = false;
    }

    arquivo.close();
    return sGrafoLido;
}

