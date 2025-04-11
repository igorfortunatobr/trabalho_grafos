#ifndef GRAFO_HPP
#define GRAFO_HPP

#include <string>
#include <vector>
#include <limits> 

using namespace std;

const int INF = numeric_limits<int>::max();

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
    string nome;
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
    vector<Vertice> vertices;
    vector<Aresta> arestas;
    vector<Arco> arcos;
};

// Funcoes utilitarias
int extrairValorInteiro(const string& linha);
double calcularDensidade(int V, int E, int A, bool ehDirecionado);
void calcularGraus(const Grafo& grafo, int V, int& grauMin, int& grauMax);
int calcularComponentesConectados(const Grafo& grafo, int V);
pair<double, int> calcularCaminhoMedioDiametro(const vector<vector<int>>& dist, int V);
vector<int> calcularIntermediacao(const vector<vector<int>>& pred, const vector<vector<int>>& dist, int V);
void exibirEstatisticasFormatadas(const Grafo& grafo, double densidade, int componentes,
    int grauMin, int grauMax, double caminhoMedio, int diametro,
    const vector<int>& intermediacao);
void salvarEmArquivo(const Grafo& grafo, double densidade, int componentes,
    int grauMin, int grauMax, double caminhoMedio, int diametro,
    const vector<int>& intermediacao, const string& nomeArquivo);

#endif