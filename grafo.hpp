#ifndef GRAFO_HPP
#define GRAFO_HPP

#include <string>
#include <vector>
#include <limits> 

using namespace std;

const int INF = numeric_limits<int>::max();

struct sVertice {
    int id;
    bool requerServico = false;
    int demanda = 0;
    int custoAtendimento = 0;
};

struct sAresta {
    int id;
    int origem;
    int destino;
    int custoTransito;
    int demanda;
    int custoAtendimento;
    bool requerServico = true;
};

struct sArco {
    int id;
    int origem;
    int destino;
    int custoTransito;
    int demanda;
    int custoAtendimento;
    bool requerServico = true;
};

struct sVizinho {
    int destino;
    int custo;
};

struct sGrafo {
    string nome;
    int valorOtimo;
    int numVeiculos;
    int capacidadeVeiculo;
    int deposito;
    int iNumVertices;
    int numArestas;
    int numArcos;
    int numVerticesRequeridos;
    int numArestasRequeridas;
    int numArcosRequeridos;
    vector<sVertice> vsVertices;
    vector<sAresta> vsArestas;
    vector<sArco> vsArcos;

    vector<sVizinho> obterAdjacentes(int idOrigem) const {
        vector<sVizinho> vizinhos;

        for (const auto& aresta : vsArestas) {
            if (aresta.origem == idOrigem)
                vizinhos.push_back({aresta.destino, aresta.custoTransito});
            
            if (aresta.destino == idOrigem)
                vizinhos.push_back({aresta.origem, aresta.custoTransito});
        }

        for (const auto& arco : vsArcos) {
            if (arco.origem == idOrigem)
                vizinhos.push_back({arco.destino, arco.custoTransito});
        }

        return vizinhos;
    }
};

// Funcoes utilitarias
int extrairValorInteiro(const string& linha);
double dCalcularDensidade(int V, int E, int A, bool ehDirecionado);
void calcularGraus(const sGrafo& grafo, int V, int& grauMin, int& grauMax);
int iCalcularComponentesConectados(const sGrafo& grafo, int V);
pair<double, int> calcularCaminhoMedioDiametro(const vector<vector<int>>& dist, int V);
vector<int> viCalcularIntermediacoes(const vector<vector<int>>& pred, const vector<vector<int>>& dist, int V);
void exibirEstatisticasFormatadas(const sGrafo& grafo, double densidade, int componentes,
    int grauMin, int grauMax, double caminhoMedio, int diametro,
    const vector<int>& intermediacao);
void salvarEmArquivo(const sGrafo& grafo, double densidade, int componentes,
    int grauMin, int grauMax, double caminhoMedio, int diametro,
    const vector<int>& intermediacao, const string& nomeArquivo);

#endif
