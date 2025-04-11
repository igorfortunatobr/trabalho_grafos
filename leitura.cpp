#include "grafo.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

int extrairValorInteiro(const string& linha) {
    size_t pos = linha.find(":");
    if (pos == string::npos) return 0;
    string numero = linha.substr(pos + 1);
    numero.erase(0, numero.find_first_not_of(" \t"));
    try {
        return stoi(numero);
    } catch (...) {
        return 0;
    }
}

Grafo lerArquivo(const string& nomeArquivo) {
    ifstream arquivo("./instancias/" + nomeArquivo);
    Grafo grafo;
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        return grafo;
    }

    string linha;
    getline(arquivo, linha); grafo.nome = linha.substr(linha.find(":") + 1);
    grafo.nome = grafo.nome.substr(grafo.nome.find("\t\t") + 2);

    getline(arquivo, linha); grafo.valorOtimo = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.numVeiculos = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.capacidadeVeiculo = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.deposito = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.numVertices = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.numArestas = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.numArcos = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.numVerticesRequeridos = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.numArestasRequeridas = extrairValorInteiro(linha);
    getline(arquivo, linha); grafo.numArcosRequeridos = extrairValorInteiro(linha);

    getline(arquivo, linha); getline(arquivo, linha); // pular linha
    while (getline(arquivo, linha) && !linha.empty()) {
        stringstream ss(linha);
        string idTexto; int demanda, custo;
        ss >> idTexto >> demanda >> custo;
        if (idTexto.empty()) continue;
        int id = stoi(idTexto.substr(1));
        grafo.vertices.push_back({id, true, demanda, custo});
    }

    getline(arquivo, linha);
    while (getline(arquivo, linha) && !linha.empty()) {
        stringstream ss(linha);
        string idTexto;
        int origem, destino, custoTransito, demanda, custoAtendimento;
        ss >> idTexto >> origem >> destino >> custoTransito >> demanda >> custoAtendimento;
        if (idTexto.empty()) continue;
        grafo.arestas.push_back({stoi(idTexto.substr(1)), origem, destino, custoTransito, demanda, custoAtendimento});
    }

    getline(arquivo, linha);
    while (getline(arquivo, linha) && !linha.empty()) {
        stringstream ss(linha);
        string idTexto;
        int origem, destino, custoTransito;
        ss >> idTexto >> origem >> destino >> custoTransito;
        if (idTexto.substr(0, 3) != "NrE") continue;
        string numeroStr = idTexto.substr(3);
        if (numeroStr.empty()) continue;
        grafo.arestas.push_back({stoi(numeroStr), origem, destino, custoTransito, 0, 0, false});
    }

    getline(arquivo, linha);
    while (getline(arquivo, linha) && !linha.empty()) {
        stringstream ss(linha);
        string idTexto;
        int origem, destino, custoTransito, demanda, custoAtendimento;
        ss >> idTexto >> origem >> destino >> custoTransito >> demanda >> custoAtendimento;
        if (idTexto.empty()) continue;
        grafo.arcos.push_back({stoi(idTexto.substr(1)), origem, destino, custoTransito, demanda, custoAtendimento, true});
    }

    getline(arquivo, linha);
    while (getline(arquivo, linha)) {
        if (linha.empty()) continue;
        stringstream ss(linha);
        string idTexto;
        int origem, destino, custoTransito;
        ss >> idTexto >> origem >> destino >> custoTransito;
        if (idTexto.size() < 4) continue;
        string numeroStr = idTexto.substr(3);
        if (numeroStr.empty()) continue;
        grafo.arcos.push_back({stoi(numeroStr), origem, destino, custoTransito, 0, 0, false});
    }

    return grafo;
}