#include "grafo.hpp"
#include <iostream>
#include <vector>
#include <string>
#include "leitura.cpp"
#include "estatisticas.cpp"

using namespace std;

void exibirMatrizDist(const vector<vector<int>>& dist, int V) {
    cout << "\nMatriz de distancias (dist[i][j]):\n";
    for (int i = 1; i <= V; ++i) {
        for (int j = 1; j <= V; ++j) {
            if (dist[i][j] == INF)
                cout << "INF\t";
            else
                cout << dist[i][j] << "\t";
        }
        cout << endl;
    }
}

void exibirMatrizPred(const vector<vector<int>>& pred, int V) {
    cout << "\nMatriz de predecessores (pred[i][j]):\n";
    for (int i = 1; i <= V; ++i) {
        for (int j = 1; j <= V; ++j) {
            if (pred[i][j] == -1)
                cout << "NULO\t";
            else
                cout << pred[i][j] << "\t";
        }
        cout << endl;
    }
}

void floydWarshall(Grafo& grafo, int N, vector<vector<int>>& dist, vector<vector<int>>& pred) {
	for (int i = 1; i < N; ++i) {
            dist[i][i] = 0;
            pred[i][i] = i;
	}

	for (const Aresta& a : grafo.arestas) {
		dist[a.origem][a.destino] = a.custoTransito;
		dist[a.destino][a.origem] = a.custoTransito;
		pred[a.origem][a.destino] = a.origem;
		pred[a.destino][a.origem] = a.destino;
	}

	for (const Arco& a : grafo.arcos) {
		dist[a.origem][a.destino] = a.custoTransito;
		pred[a.origem][a.destino] = a.origem;
	}
	
	for (int k = 1; k < N; ++k) {
            for (int i = 1; i < N; ++i) {
                for (int j = 1; j < N; ++j) {
                    if (dist[i][k] != INF && dist[k][j] != INF && dist[i][k] + dist[k][j] < dist[i][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        pred[i][j] = pred[k][j];
                    }
                }
            }
        }
}


int main() {
    string nomeArquivo;
    cout << "Digite o nome do arquivo: ";
	cin >> nomeArquivo;
    while (nomeArquivo != "") {

        Grafo grafo = lerArquivo(nomeArquivo);

        bool ehDirecionado = !grafo.arcos.empty();
        int N = grafo.numVertices + 1;

        vector<vector<int>> dist(N, vector<int>(N, INF));
        vector<vector<int>> pred(N, vector<int>(N, -1));

        floydWarshall(grafo, N, dist, pred);

		exibirMatrizDist(dist, grafo.numVertices);
		exibirMatrizPred(pred, grafo.numVertices);

        double densidade = calcularDensidade(grafo.numVertices, grafo.arestas.size(), grafo.arcos.size(), ehDirecionado);
        int grauMin, grauMax;
        calcularGraus(grafo, grafo.numVertices, grauMin, grauMax);
        int componentes = calcularComponentesConectados(grafo, grafo.numVertices);
        pair<double, int> res = calcularCaminhoMedioDiametro(dist, grafo.numVertices);
        vector<int> intermediacao = calcularIntermediacao(pred, dist, grafo.numVertices);

        exibirEstatisticasFormatadas(grafo, densidade, componentes, grauMin, grauMax, res.first, res.second, intermediacao);

        string saida = "estatisticas/estatisticas_" + grafo.nome + ".json";
        salvarEmArquivo(grafo, densidade, componentes, grauMin, grauMax, res.first, res.second, intermediacao, saida, dist, pred);

        cout << "\nDigite o nome do arquivo: ";
		cin >> nomeArquivo;
    }
    return 0;
}
