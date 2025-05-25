#include "grafo.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept> // Para std::runtime_error

using namespace std;

// Função auxiliar para extrair valor inteiro de uma linha "CHAVE : VALOR"
int extrairValorInteiro(const string& linha, const string& chave) {
    size_t pos = linha.find(":");
    if (pos == string::npos) {
        cerr << "DEBUG: Formato invalido (sem ':') na linha para chave '" << chave << "': " << linha << endl;
        return 0; // Ou lançar exceção
    }
    string valorStr = linha.substr(pos + 1);
    // Remover espaços em branco antes do número
    valorStr.erase(0, valorStr.find_first_not_of(" \t"));
    try {
        return stoi(valorStr);
    } catch (const std::invalid_argument& ia) {
        cerr << "DEBUG: Erro ao converter para inteiro (invalid_argument) na linha para chave '" << chave << "': " << linha << " | Valor string: '" << valorStr << "'" << endl;
        return 0; // Ou lançar exceção
    } catch (const std::out_of_range& oor) {
        cerr << "DEBUG: Erro ao converter para inteiro (out_of_range) na linha para chave '" << chave << "': " << linha << " | Valor string: '" << valorStr << "'" << endl;
        return 0; // Ou lançar exceção
    }
}

Grafo lerArquivo(const string& nomeArquivo, bool& conseguiuLerArquivo) {
    conseguiuLerArquivo = false; // Inicializa como falso
    Grafo grafo;
    cout << "DEBUG: Tentando abrir arquivo: " << nomeArquivo << endl;
    ifstream arquivo("./instancias/" + nomeArquivo);

    if (!arquivo.is_open()) {
        cerr << "DEBUG: Falha ao abrir o arquivo: " << nomeArquivo << endl;
        return grafo; // Retorna grafo vazio, conseguiuLerArquivo continua false
    }
    cout << "DEBUG: Arquivo aberto com sucesso: " << nomeArquivo << endl;

    string linha;
    try {
        // Leitura do cabeçalho
        cout << "DEBUG: Lendo cabecalho..." << endl;
        getline(arquivo, linha); grafo.nome = linha.substr(linha.find(":") + 1);
        grafo.nome = grafo.nome.substr(grafo.nome.find_first_not_of(" \t")); // Remover espaços iniciais
        cout << "DEBUG: Nome: " << grafo.nome << endl;

        getline(arquivo, linha); grafo.valorOtimo = extrairValorInteiro(linha, "VALOR OTIMO");
        getline(arquivo, linha); grafo.numVeiculos = extrairValorInteiro(linha, "NUMERO DE VEICULOS");
        getline(arquivo, linha); grafo.capacidadeVeiculo = extrairValorInteiro(linha, "CAPACIDADE DOS VEICULOS");
        getline(arquivo, linha); grafo.deposito = extrairValorInteiro(linha, "DEPOSITO");
        getline(arquivo, linha); grafo.numVertices = extrairValorInteiro(linha, "NUMERO DE VERTICES");
        getline(arquivo, linha); grafo.numArestas = extrairValorInteiro(linha, "NUMERO DE ARESTAS");
        getline(arquivo, linha); grafo.numArcos = extrairValorInteiro(linha, "NUMERO DE ARCOS");
        getline(arquivo, linha); grafo.numVerticesRequeridos = extrairValorInteiro(linha, "NUMERO DE VERTICES REQUERIDOS");
        getline(arquivo, linha); grafo.numArestasRequeridas = extrairValorInteiro(linha, "NUMERO DE ARESTAS REQUERIDAS");
        getline(arquivo, linha); grafo.numArcosRequeridos = extrairValorInteiro(linha, "NUMERO DE ARCOS REQUERIDOS");
        cout << "DEBUG: Cabecalho lido. Vertices: " << grafo.numVertices << endl;

        // Pular linhas de seção
        getline(arquivo, linha); // VERTICES REQUERIDOS
        cout << "DEBUG: Secao: " << linha << endl;
        getline(arquivo, linha); // (id demanda custo)
        cout << "DEBUG: Cabecalho secao: " << linha << endl;

        // Ler vértices requeridos
        cout << "DEBUG: Lendo vertices requeridos..." << endl;
        while (getline(arquivo, linha) && !linha.empty() && linha.find("ARESTAS REQUERIDAS") == string::npos) {
            stringstream ss(linha);
            string idTexto; int demanda, custo;
            ss >> idTexto >> demanda >> custo;
            if (idTexto.empty() || idTexto[0] != 'N') continue; // Ignora linhas vazias ou mal formatadas
            int id = stoi(idTexto.substr(1));
            grafo.vertices.push_back({id, true, demanda, custo});
            cout << "DEBUG: Vertice Requerido Lido: ID=" << id << ", Dem=" << demanda << ", Custo=" << custo << endl;
        }
        cout << "DEBUG: Terminou de ler vertices requeridos. Linha atual: " << linha << endl;

        // Ler arestas requeridas
        cout << "DEBUG: Lendo arestas requeridas..." << endl;
        getline(arquivo, linha); // (id orig dest custo_transito demanda custo_atendimento)
        cout << "DEBUG: Cabecalho secao: " << linha << endl;
        while (getline(arquivo, linha) && !linha.empty() && linha.find("ARESTAS NAO REQUERIDAS") == string::npos) {
            stringstream ss(linha);
            string idTexto;
            int origem, destino, custoTransito, demanda, custoAtendimento;
            ss >> idTexto >> origem >> destino >> custoTransito >> demanda >> custoAtendimento;
            if (idTexto.empty() || idTexto[0] != 'E') continue;
            grafo.arestas.push_back({stoi(idTexto.substr(1)), origem, destino, custoTransito, demanda, custoAtendimento, true});
             cout << "DEBUG: Aresta Requerida Lida: ID=" << idTexto.substr(1) << endl;
        }
         cout << "DEBUG: Terminou de ler arestas requeridas. Linha atual: " << linha << endl;

        // Ler arestas não requeridas
        cout << "DEBUG: Lendo arestas nao requeridas..." << endl;
        getline(arquivo, linha); // (id orig dest custo_transito)
        cout << "DEBUG: Cabecalho secao: " << linha << endl;
        while (getline(arquivo, linha) && !linha.empty() && linha.find("ARCOS REQUERIDOS") == string::npos) {
            stringstream ss(linha);
            string idTexto;
            int origem, destino, custoTransito;
            ss >> idTexto >> origem >> destino >> custoTransito;
            if (idTexto.empty() || idTexto.substr(0, 3) != "NrE") continue;
            grafo.arestas.push_back({stoi(idTexto.substr(3)), origem, destino, custoTransito, 0, 0, false});
             cout << "DEBUG: Aresta Nao Requerida Lida: ID=" << idTexto.substr(3) << endl;
        }
         cout << "DEBUG: Terminou de ler arestas nao requeridas. Linha atual: " << linha << endl;

        // Ler arcos requeridos
        cout << "DEBUG: Lendo arcos requeridos..." << endl;
        getline(arquivo, linha); // (id orig dest custo_transito demanda custo_atendimento)
        cout << "DEBUG: Cabecalho secao: " << linha << endl;
        while (getline(arquivo, linha) && !linha.empty() && linha.find("ARCOS NAO REQUERIDOS") == string::npos) {
            stringstream ss(linha);
            string idTexto;
            int origem, destino, custoTransito, demanda, custoAtendimento;
            ss >> idTexto >> origem >> destino >> custoTransito >> demanda >> custoAtendimento;
            if (idTexto.empty() || idTexto[0] != 'A') continue;
            grafo.arcos.push_back({stoi(idTexto.substr(1)), origem, destino, custoTransito, demanda, custoAtendimento, true});
             cout << "DEBUG: Arco Requerido Lido: ID=" << idTexto.substr(1) << endl;
        }
         cout << "DEBUG: Terminou de ler arcos requeridos. Linha atual: " << linha << endl;

        // Ler arcos não requeridos
        cout << "DEBUG: Lendo arcos nao requeridos..." << endl;
        getline(arquivo, linha); // (id orig dest custo_transito)
        cout << "DEBUG: Cabecalho secao: " << linha << endl;
        while (getline(arquivo, linha) && !linha.empty() && linha.find("FIM") == string::npos) { // Ler até FIM
            stringstream ss(linha);
            string idTexto;
            int origem, destino, custoTransito;
            ss >> idTexto >> origem >> destino >> custoTransito;
            if (idTexto.empty() || idTexto.substr(0, 3) != "NrA") continue;
            grafo.arcos.push_back({stoi(idTexto.substr(3)), origem, destino, custoTransito, 0, 0, false});
             cout << "DEBUG: Arco Nao Requerido Lido: ID=" << idTexto.substr(3) << endl;
        }
         cout << "DEBUG: Terminou de ler arcos nao requeridos. Linha atual: " << linha << endl;

        // Se chegou até aqui sem exceções, a leitura foi bem-sucedida
        conseguiuLerArquivo = true;
        cout << "DEBUG: Leitura concluida com sucesso para " << nomeArquivo << endl;

    } catch (const std::exception& e) {
        cerr << "DEBUG: Excecao durante a leitura do arquivo " << nomeArquivo << ": " << e.what() << endl;
        conseguiuLerArquivo = false;
    }

    arquivo.close();
    return grafo;
}

