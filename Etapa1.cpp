#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct Vertice {
    int id;
    bool requerServico = false;
    int demanda = 0;
    int custoAtendimento = 0;
};

struct Aresta {
    int id;
    int origem;
    int destino;
    int custoTransito;
    int demanda;
    int custoAtendimento;
    bool requerServico = true;
};

struct Arco {
    int id;
    int origem;
    int destino;
    int custoTransito;
    int demanda;
    int custoAtendimento;
    bool requerServico = true;
};

struct Grafo {
    std::string nome;
    int valorOtimo;
    int numVeiculos;
    int capacidadeVeiculo;
    int deposito;
    int numVertices;
    int numArestas;
    int numArcos;
    int numVerticesRequeridos;
    int numArestasRequeridas;
    int numArcosRequeridos;

    std::vector<Vertice> vertices;
    std::vector<Aresta> arestas;
    std::vector<Arco> arcos;
};

// Funcao robusta para extrair numero inteiro depois de ":"
int extrairValorInteiro(const std::string& linha) {
    size_t pos = linha.find(":");
    if (pos == std::string::npos) return 0;

    std::string numero = linha.substr(pos + 1);
    numero.erase(0, numero.find_first_not_of(" \t")); // remove espacos e tabs

    if (numero.empty()) return 0;

    try {
        return std::stoi(numero);
    } catch (...) {
        return 0;
    }
}

int main() {
    //std::ifstream arquivo("DI-NEARP-n240-Q2k.dat");
    std::ifstream arquivo("BHW1.dat");

    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return 1;
    }

    std::string linha;
    Grafo grafo;

    try {
        getline(arquivo, linha); grafo.nome = linha.substr(linha.find(":") + 1);
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
    } catch (...) {
        std::cerr << "Erro ao ler informacoes do cabecalho." << std::endl;
        return 1;
    }

    getline(arquivo, linha); // linha em branco
    getline(arquivo, linha); // cabecalho ReN.

    while (getline(arquivo, linha) && !linha.empty()) {
        std::stringstream ss(linha);
        std::string idTexto;
        int demanda, custo;
        ss >> idTexto >> demanda >> custo;
        if (idTexto.empty()) continue;
        int id = std::stoi(idTexto.substr(1)); // remove 'N'
        grafo.vertices.push_back({id, true, demanda, custo});
    }

    getline(arquivo, linha); // cabecalho ReE.

    while (getline(arquivo, linha) && !linha.empty()) {
        std::stringstream ss(linha);
        std::string idTexto;
        int origem, destino, custoTransito, demanda, custoAtendimento;
        ss >> idTexto >> origem >> destino >> custoTransito >> demanda >> custoAtendimento;
        if (idTexto.empty()) continue;
        grafo.arestas.push_back({std::stoi(idTexto.substr(1)), origem, destino, custoTransito, demanda, custoAtendimento});
    }

    getline(arquivo, linha); // linha EDGE...

    // Leitura das arestas nao obrigatorias (NrE)
    while (getline(arquivo, linha) && !linha.empty()) {
        std::stringstream ss(linha);
        std::string idTexto;
        int origem, destino, custoTransito;
        ss >> idTexto >> origem >> destino >> custoTransito;

        if (idTexto.empty() || idTexto.substr(0, 3) != "NrE") continue;

        std::string numeroStr = idTexto.substr(3);
        if (numeroStr.empty()) continue;

        try {
            int id = std::stoi(numeroStr);
            grafo.arestas.push_back({id, origem, destino, custoTransito, 0, 0, false});
        } catch (...) {
            std::cerr << "Erro ao converter ID de aresta nao obrigatoria: " << idTexto << std::endl;
            continue;
        }
    }

    getline(arquivo, linha); // cabecalho ReA.

    while (getline(arquivo, linha) && !linha.empty()) {
        std::stringstream ss(linha);
        std::string idTexto;
        int origem, destino, custoTransito, demanda, custoAtendimento;
        ss >> idTexto >> origem >> destino >> custoTransito >> demanda >> custoAtendimento;
        if (idTexto.empty()) continue;
        grafo.arcos.push_back({std::stoi(idTexto.substr(1)), origem, destino, custoTransito, demanda, custoAtendimento, true});
    }

    getline(arquivo, linha); // cabecalho ARC...

    while (getline(arquivo, linha)) {
        if (linha.empty()) continue;

        std::stringstream ss(linha);
        std::string idTexto;
        int origem, destino, custoTransito;
        ss >> idTexto >> origem >> destino >> custoTransito;

        if (idTexto.size() < 4) continue;
        std::string numeroStr = idTexto.substr(3);
        if (numeroStr.empty()) continue;

        try {
            int id = std::stoi(numeroStr);
            grafo.arcos.push_back({id, origem, destino, custoTransito, 0, 0, false});
        } catch (...) {
            continue;
        }
    }

    // Impressao dos dados lidos
    std::cout << "Arquivo lido com sucesso!" << std::endl;
    std::cout << "Nome: " << grafo.nome << std::endl;
    std::cout << "Valor otimo: " << grafo.valorOtimo << std::endl;
    std::cout << "Veiculos: " << grafo.numVeiculos << std::endl;
    std::cout << "Capacidade: " << grafo.capacidadeVeiculo << std::endl;
    std::cout << "Deposito: " << grafo.deposito << std::endl;
    std::cout << "Vertices totais: " << grafo.numVertices << ", requeridos: " << grafo.numVerticesRequeridos << std::endl;
    std::cout << "Arestas totais: " << grafo.numArestas << ", requeridas: " << grafo.numArestasRequeridas << std::endl;
    std::cout << "Arcos totais: " << grafo.numArcos << ", requeridos: " << grafo.numArcosRequeridos << std::endl;

    int obrigatorios = 0, naoObrigatorios = 0;
    for (const Arco& a : grafo.arcos) {
        if (a.requerServico) obrigatorios++;
        else naoObrigatorios++;
    }

    std::cout << "Arcos obrigatorios: " << obrigatorios << std::endl;
    std::cout << "Arcos nao obrigatorios: " << naoObrigatorios << std::endl;

    return 0;
}
