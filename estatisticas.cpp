#include "grafo.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

double calcularDensidade(int V, int E, int A, bool ehDirecionado) {
    if (V <= 1) return 0.0;
    return static_cast<double>((2 * E) + A) / (V * (V - 1));
}

void calcularGraus(const Grafo& grafo, int V, int& grauMin, int& grauMax) {
    vector<int> grau(V + 1, 0);
    for (const Aresta& a : grafo.arestas) {
        grau[a.origem]++;
        grau[a.destino]++;
    }
    for (const Arco& a : grafo.arcos) {
        grau[a.origem]++;
        grau[a.destino]++;
    }
    grauMin = INF;
    grauMax = -INF;
    for (int i = 1; i <= V; ++i) {
        grauMin = min(grauMin, grau[i]);
        grauMax = max(grauMax, grau[i]);
    }
}

void dfsVisit(int v, const vector<vector<int>>& adj, vector<bool>& visitado) {
    visitado[v] = true;
    for (int viz : adj[v]) {
        if (!visitado[viz]) dfsVisit(viz, adj, visitado);
    }
}

int calcularComponentesConectados(const Grafo& grafo, int V) {
    vector<vector<int>> adj(V + 1);
    for (const Aresta& a : grafo.arestas) {
        adj[a.origem].push_back(a.destino);
        adj[a.destino].push_back(a.origem);
    }
    for (const Arco& a : grafo.arcos) {
        adj[a.origem].push_back(a.destino);
        adj[a.destino].push_back(a.origem);
    }
    vector<bool> visitado(V + 1, false);
    int componentes = 0;
    for (int i = 1; i <= V; ++i) {
        if (!visitado[i]) {
            dfsVisit(i, adj, visitado);
            componentes++;
        }
    }
    return componentes;
}

pair<double, int> calcularCaminhoMedioDiametro(const vector<vector<int>>& dist, int V) {
    int total = 0, pares = 0, diametro = 0;
    for (int i = 1; i <= V; ++i) {
        for (int j = 1; j <= V; ++j) {
            if (i != j && dist[i][j] != INF) {
                total += dist[i][j];
                pares++;
                diametro = max(diametro, dist[i][j]);
            }
        }
    }
    double caminhoMedio = (pares > 0) ? static_cast<double>(total) / pares : 0.0;
    return {caminhoMedio, diametro};
}

vector<int> calcularIntermediacao(const vector<vector<int>>& pred, const vector<vector<int>>& dist, int V) {
    vector<int> inter(V + 1, 0);
    for (int s = 1; s <= V; ++s) {
        for (int t = 1; t <= V; ++t) {
            if (s != t && dist[s][t] != INF) {
                int atual = t;
                while (pred[s][atual] != -1 && pred[s][atual] != s) {
                    inter[pred[s][atual]]++;
                    atual = pred[s][atual];
                }
            }
        }
    }
    return inter;
}

string ajustarJSON(const string& jsonEntrada) {
    string jsonSaida;
    for (char c : jsonEntrada) {
        if (c == '"') jsonSaida += "\\\"";
        else if (c == '\\') jsonSaida += "\\\\";
        else if (c == '\n') jsonSaida += "\\n";
        else if (c == '\r') jsonSaida += "\\r";
        else if (c == '\t') jsonSaida += "\\t";
        else jsonSaida.push_back(c);
    }
    return jsonSaida;
}

string gerarGrafoJSON(const Grafo &grafo) {
    stringstream ss;
    ss << "{\n";
    
    // Lista de vértices
    ss << "\t\"vertices\": [\n";
    for (size_t i = 0; i < grafo.vertices.size(); i++) {
        ss << "\t\t{ \"id\": " << grafo.vertices[i].id << " }";
        if (i != grafo.vertices.size() - 1)
            ss << ",";
        ss << "\n";
    }
    ss << "\t],\n";
    
    // Lista de arestas (caminhos não direcionados)
    ss << "\t\"arestas\": [\n";
    for (size_t i = 0; i < grafo.arestas.size(); i++) {
        ss << "\t\t{ \"origem\": " << grafo.arestas[i].origem 
           << ", \"destino\": " << grafo.arestas[i].destino 
           << ", \"custoTransito\": " << grafo.arestas[i].custoTransito << " }";
        if (i != grafo.arestas.size() - 1)
            ss << ",";
        ss << "\n";
    }
    ss << "\t],\n";
    
    // Lista de arcos (caminhos direcionados)
    ss << "\t\"arcos\": [\n";
    for (size_t i = 0; i < grafo.arcos.size(); i++) {
        ss << "\t\t{ \"origem\": " << grafo.arcos[i].origem 
           << ", \"destino\": " << grafo.arcos[i].destino 
           << ", \"custoTransito\": " << grafo.arcos[i].custoTransito << " }";
        if (i != grafo.arcos.size() - 1)
            ss << ",";
        ss << "\n";
    }
    ss << "\t]\n";
    ss << "}\n";
    return ss.str();
}

void exibirEstatisticasFormatadas(const Grafo& grafo, double densidade, int componentes,
    int grauMin, int grauMax, double caminhoMedio, int diametro,
    const vector<int>& intermediacao) {

    cout << "\n=== Estatisticas ===\n";
    cout << "1. Quantidade de vertices: " << grafo.numVertices << endl;
    cout << "2. Quantidade de arestas: " << grafo.arestas.size() << endl;
    cout << "3. Quantidade de arcos: " << grafo.arcos.size() << endl;
    cout << "4. Quantidade de vertices requeridos: " << grafo.numVerticesRequeridos << endl;
    cout << "5. Quantidade de arestas requeridas: " << grafo.numArestasRequeridas << endl;
    cout << "6. Quantidade de arcos requeridos: " << grafo.numArcosRequeridos << endl;
    cout << "7. Densidade do grafo (order strength): " << densidade << endl;
    cout << "8. Componentes conectados: " << componentes << endl;
    cout << "9. Grau minimo dos vertices: " << grauMin << endl;
    cout << "10. Grau maximo dos vertices: " << grauMax << endl;
    cout << "11. Intermediacao:" << endl;
    for (int i = 1; i <= grafo.numVertices; ++i) {
        cout << "Vertice " << i << ": " << intermediacao[i] << endl;
    }
    cout << "12. Caminho medio: " << caminhoMedio << endl;
    cout << "13. Diametro: " << diametro << endl;
}

void salvarEmArquivo(const Grafo& grafo, double densidade, int componentes,
    int grauMin, int grauMax, double caminhoMedio, int diametro,
    const vector<int>& intermediacao, const string& nomeArquivo) {

    ofstream arquivoSaida(nomeArquivo);
    if (!arquivoSaida.is_open()) {
        cerr << "Erro ao abrir o arquivo de saída!" << endl;
        return;
    }
    string grafoString = gerarGrafoJSON(grafo);
    string jsonAjustado = ajustarJSON(grafoString);

    arquivoSaida << "{\n";
    arquivoSaida << "  \"estatisticas\": {\n";
    arquivoSaida << "    \"nome\": \"" << grafo.nome << "\",\n";
    arquivoSaida << "    \"numVertices\": " << grafo.numVertices << ",\n";
    arquivoSaida << "    \"numArestas\": " << grafo.arestas.size() << ",\n";
    arquivoSaida << "    \"numArcos\": " << grafo.arcos.size() << ",\n";
    arquivoSaida << "    \"numVerticesRequeridos\": " << grafo.numVerticesRequeridos << ",\n";
    arquivoSaida << "    \"numArestasRequeridas\": " << grafo.numArestasRequeridas << ",\n";
    arquivoSaida << "    \"numArcosRequeridos\": " << grafo.numArcosRequeridos << ",\n";
    arquivoSaida << "    \"densidade\": " << densidade << ",\n";
    arquivoSaida << "    \"componentes\": " << componentes << ",\n";
    arquivoSaida << "    \"grauMin\": " << grauMin << ",\n";
    arquivoSaida << "    \"grauMax\": " << grauMax << ",\n";
    arquivoSaida << "    \"caminhoMedio\": " << caminhoMedio << ",\n";
    arquivoSaida << "    \"diametro\": " << diametro << ",\n";
    arquivoSaida << "    \"intermediacao\": [";
    for (size_t i = 1; i < intermediacao.size(); ++i) {
        arquivoSaida << intermediacao[i];
        if (i < intermediacao.size() - 1) arquivoSaida << ", ";
    }
    arquivoSaida << "]\n";
    arquivoSaida << "  },\n";
    arquivoSaida << "  \"grafo_json\": \"" << jsonAjustado << "\"\n";
    arquivoSaida << "}";
    arquivoSaida.close();
    cout << "\nInformacoes salvas em: " << nomeArquivo << endl;
}
