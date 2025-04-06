#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;

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

// Função para extrair número após ":"
int extrairValorInteiro(const string& linha) {
    size_t pos = linha.find(":");
    if (pos == string::npos) return 0;
    string numero = linha.substr(pos + 1);
    numero.erase(0, numero.find_first_not_of(" \t"));
    if (numero.empty()) return 0;
    try {
        return stoi(numero);
    } catch (...) {
        return 0;
    }
}

// Densidade
double calcularDensidade(int V, int E, int A) {
    return (double)(2 * E + A) / (V * (V - 1));
}

// Grau mínimo e máximo
pair<int, int> calcularGraus(int V, const Grafo& grafo) {
    vector<int> grau(V + 1, 0);
    for (const Aresta& a : grafo.arestas) {
        grau[a.origem]++;
        grau[a.destino]++;
    }
    for (const Arco& a : grafo.arcos) {
        grau[a.origem]++;
        grau[a.destino]++;
    }

    int grauMin = numeric_limits<int>::max();
    int grauMax = numeric_limits<int>::min();
    for (int i = 1; i <= V; ++i) {
        grauMin = min(grauMin, grau[i]);
        grauMax = max(grauMax, grau[i]);
    }

    return {grauMin, grauMax};
}

// DFS recursivo para componentes conectados
void dfsComponentes(int v, vector<bool>& visitado, const vector<vector<int>>& adj) {
    visitado[v] = true;
    for (int viz : adj[v]) {
        if (!visitado[viz]) {
            dfsComponentes(viz, visitado, adj);
        }
    }
}

int calcularComponentesConectados(int V, const Grafo& grafo) {
    vector<bool> visitado(V + 1, false);
    vector<vector<int>> adj(V + 1);
    for (const Aresta& a : grafo.arestas) {
        adj[a.origem].push_back(a.destino);
        adj[a.destino].push_back(a.origem);
    }
    for (const Arco& a : grafo.arcos) {
        adj[a.origem].push_back(a.destino);
        adj[a.destino].push_back(a.origem);
    }

    int componentes = 0;
    for (int i = 1; i <= V; ++i) {
        if (!visitado[i]) {
            dfsComponentes(i, visitado, adj);
            componentes++;
        }
    }
    return componentes;
}

// Caminho médio e diâmetro
pair<double, int> calcularCaminhoMedioDiametro(const vector<vector<int>>& dist, int V) {
    const int INF = numeric_limits<int>::max();
    int totalDistancia = 0;
    int paresValidos = 0;
    int diametro = 0;

    for (int i = 1; i <= V; ++i) {
        for (int j = 1; j <= V; ++j) {
            if (i != j && dist[i][j] != INF) {
                totalDistancia += dist[i][j];
                paresValidos++;
                diametro = max(diametro, dist[i][j]);
            }
        }
    }

    double caminhoMedio = paresValidos > 0 ? (double)totalDistancia / paresValidos : 0;
    return {caminhoMedio, diametro};
}

// Intermediação
vector<int> calcularIntermediacao(const vector<vector<int>>& pred, const vector<vector<int>>& dist, int V) {
    const int INF = numeric_limits<int>::max();
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

int main() {
    ifstream arquivo("TESTE.dat");
    if (!arquivo.is_open()) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        return 1;
    }

    string linha;
    Grafo grafo;

    // Leitura do cabeçalho
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
        cerr << "Erro ao ler informacoes do cabecalho." << endl;
        return 1;
    }

    // Leitura das seções
    getline(arquivo, linha); getline(arquivo, linha); // pula em branco + ReN
    while (getline(arquivo, linha) && !linha.empty()) {
        stringstream ss(linha);
        string idTexto; int demanda, custo;
        ss >> idTexto >> demanda >> custo;
        if (idTexto.empty()) continue;
        int id = stoi(idTexto.substr(1));
        grafo.vertices.push_back({id, true, demanda, custo});
    }

    getline(arquivo, linha); // ReE
    while (getline(arquivo, linha) && !linha.empty()) {
        stringstream ss(linha);
        string idTexto;
        int origem, destino, custoTransito, demanda, custoAtendimento;
        ss >> idTexto >> origem >> destino >> custoTransito >> demanda >> custoAtendimento;
        if (idTexto.empty()) continue;
        grafo.arestas.push_back({stoi(idTexto.substr(1)), origem, destino, custoTransito, demanda, custoAtendimento});
    }

    getline(arquivo, linha); // NrE
    while (getline(arquivo, linha) && !linha.empty()) {
        stringstream ss(linha);
        string idTexto; int origem, destino, custoTransito;
        ss >> idTexto >> origem >> destino >> custoTransito;
        if (idTexto.substr(0, 3) != "NrE") continue;
        string numeroStr = idTexto.substr(3);
        if (numeroStr.empty()) continue;
        grafo.arestas.push_back({stoi(numeroStr), origem, destino, custoTransito, 0, 0, false});
    }

    getline(arquivo, linha); // ReA
    while (getline(arquivo, linha) && !linha.empty()) {
        stringstream ss(linha);
        string idTexto; int origem, destino, custoTransito, demanda, custoAtendimento;
        ss >> idTexto >> origem >> destino >> custoTransito >> demanda >> custoAtendimento;
        if (idTexto.empty()) continue;
        grafo.arcos.push_back({stoi(idTexto.substr(1)), origem, destino, custoTransito, demanda, custoAtendimento, true});
    }

    getline(arquivo, linha); // Arc
    while (getline(arquivo, linha)) {
        if (linha.empty()) continue;
        stringstream ss(linha);
        string idTexto; int origem, destino, custoTransito;
        ss >> idTexto >> origem >> destino >> custoTransito;
        if (idTexto.size() < 4) continue;
        string numeroStr = idTexto.substr(3);
        if (numeroStr.empty()) continue;
        try {
            int id = stoi(numeroStr);
            grafo.arcos.push_back({id, origem, destino, custoTransito, 0, 0, false});
        } catch (...) { continue; }
    }

    // Matriz de Floyd-Warshall
    const int INF = numeric_limits<int>::max();
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

    for (int k = 1; k < N; ++k)
        for (int i = 1; i < N; ++i)
            for (int j = 1; j < N; ++j)
                if (dist[i][k] != INF && dist[k][j] != INF && dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    pred[i][j] = pred[k][j];
                }

    // Impressão dos dados
    cout << "\nArquivo lido com sucesso!" << endl;
    cout << "Nome: " << grafo.nome << endl;
    cout << "Valor otimo: " << grafo.valorOtimo << endl;
    cout << "Veiculos: " << grafo.numVeiculos << endl;
    cout << "Capacidade: " << grafo.capacidadeVeiculo << endl;
    cout << "Deposito: " << grafo.deposito << endl;
    cout << "Vertices totais: " << grafo.numVertices << ", requeridos: " << grafo.numVerticesRequeridos << endl;
    cout << "Arestas totais: " << grafo.numArestas << ", requeridas: " << grafo.numArestasRequeridas << endl;
    cout << "Arcos totais: " << grafo.numArcos << ", requeridos: " << grafo.numArcosRequeridos << endl;

    // Matrizes
    cout << "\nMatriz de caminhos mais curtos (dist[i][j]):\n";
    for (int i = 1; i < N; ++i) {
        for (int j = 1; j < N; ++j) {
            if (dist[i][j] == INF) cout << "INF ";
            else cout << dist[i][j] << " ";
        }
        cout << endl;
    }

    cout << "\nMatriz de predecessores (pred[i][j]):\n";
    for (int i = 1; i < N; ++i) {
        for (int j = 1; j < N; ++j)
            cout << pred[i][j] << " ";
        cout << endl;
    }

    // Estatísticas individuais
    int V = grafo.numVertices;
    double densidade = calcularDensidade(V, grafo.arestas.size(), grafo.arcos.size());
    pair<int, int> graus = calcularGraus(V, grafo);
    int componentes = calcularComponentesConectados(V, grafo);
    pair<double, int> mediaDiametro = calcularCaminhoMedioDiametro(dist, V);
    vector<int> intermediacao = calcularIntermediacao(pred, dist, V);

    // Exibir estatísticas
    cout << "\n=== Estatisticas ===" << endl;
    cout << "Densidade do grafo: " << densidade << endl;
    cout << "Componentes conectados: " << componentes << endl;
    cout << "Grau minimo: " << graus.first << endl;
    cout << "Grau maximo: " << graus.second << endl;

    cout << "\n=== Estatisticas Avancadas ===" << endl;
    cout << "Caminho medio: " << mediaDiametro.first << endl;
    cout << "Diametro: " << mediaDiametro.second << endl;
    cout << "Intermediacao (frequencia nos caminhos minimos):" << endl;
    for (int i = 1; i <= V; ++i) {
        cout << "Vertice " << i << ": " << intermediacao[i] << endl;
    }

    return 0;
}
