#include "grafo.hpp"
#include <iostream>
#include <vector>
#include <string>
#include "leitura.cpp"
#include "estatisticas.cpp"

using namespace std;

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

        double densidade = calcularDensidade(grafo.numVertices, grafo.arestas.size(), grafo.arcos.size(), ehDirecionado);
        int grauMin, grauMax;
        calcularGraus(grafo, grafo.numVertices, grauMin, grauMax);
        int componentes = calcularComponentesConectados(grafo, grafo.numVertices);
        pair<double, int> res = calcularCaminhoMedioDiametro(dist, grafo.numVertices);
        vector<int> intermediacao = calcularIntermediacao(pred, dist, grafo.numVertices);

        exibirEstatisticasFormatadas(grafo, densidade, componentes, grauMin, grauMax, res.first, res.second, intermediacao);

        string saida = "estatisticas/estatisticas_" + grafo.nome + ".json";
        salvarEmArquivo(grafo, densidade, componentes, grauMin, grauMax, res.first, res.second, intermediacao, saida);

        cout << "\nDigite o nome do arquivo: " << endl;
		cin >> nomeArquivo;
    }
    return 0;
}
