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

// === Funções auxiliares ===

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

double calcularDensidade(int V, int E, int A) {
    if (V <= 1) return 0.0;
    return static_cast<double>(2 * E + A) / (V * (V - 1));
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

    grauMin = numeric_limits<int>::max();
    grauMax = numeric_limits<int>::min();
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
    const int INF = numeric_limits<int>::max();
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

void exibirEstatisticasFormatadas(
    const Grafo& grafo, double densidade, int componentes,
    int grauMin, int grauMax, double caminhoMedio, int diametro,
    const vector<int>& intermediacao
) {
    cout << "\n=== Estatisticas ===" << endl;
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

// Função para traduzir caracteres especiais em caracteres permitidos pelo JSON
// (Para não ocorrer problema de formatação do JSON na hora de ler no código Python)
string ajustarJSON(const string& jsonEntrada) {
    string jsonSaida;
    for (char c : jsonEntrada) {
        if (c == '\"')
            jsonSaida += "\\\"";
        else if (c == '\\')
            jsonSaida += "\\\\";
        else if (c == '\n')
            jsonSaida += "\\n";
        else if (c == '\r')
            jsonSaida += "\\r";
        else if (c == '\t')
            jsonSaida += "\\t";
        else
            jsonSaida.push_back(c);
    }
    return jsonSaida;
}

// Gera a representação do grafo em formato DOT (Formato reconhecido pela biblioteca "pydot")
string gerarGrafoDOT(const Grafo &grafo) {
    stringstream ss;
    ss << "digraph G {" << "\n";
    for (const Vertice &v : grafo.vertices) {
        ss << "\t" << v.id << " [label=\"" << v.id << "\"];" << "\n";
    }
    for (const Aresta &a : grafo.arestas) {
        ss << "\t" << a.origem << " -> " << a.destino 
           << " [dir=both, arrowtail=none, arrowhead=none, label=\"" 
           << a.custoTransito << "\"];" << "\n";
    }
    for (const Arco &a : grafo.arcos) {
        ss << "\t" << a.origem << " -> " << a.destino 
           << " [label=\"" << a.custoTransito << "\"];" << "\n";
    }
    ss << "}" << "\n";
    return ss.str();
}

// Método que salva estatísticas e o desenho do grafo em um arquivo JSON
/*
 * Exemplo de formato de JSON esperado pelo código em python
 * {
  "estatisticas": {
    "numVertices": 2,
    "numArestas": 1,
    "numArcos": 0,
    "numVerticesRequeridos": 1,
    "numArestasRequeridas": 1,
    "numArcosRequeridos": 0,
    "densidade": 1.0,
    "componentes": 1,
    "grauMin": 1,
    "grauMax": 1,
    "caminhoMedio": 10.0,
    "diametro": 10,
    "intermediacao": [0, 0, 0]
  },
  "grafo_dot": "digraph G {\\n  1 [label=\"1\"];\\n  2 [label=\"2\"];\\n  1 -> 2 [dir=both, arrowtail=none, arrowhead=none, label=\"10\"];\\n}"
}
 * 
 * 
 * */
void salvarEmArquivo(const Grafo& grafo, double densidade, int componentes,
                          int grauMin, int grauMax, double caminhoMedio, int diametro,
                          const vector<int>& intermediacao, const string& nomeArquivo) {
    ofstream arquivoSaida(nomeArquivo);
    if (!arquivoSaida.is_open()) {
        cerr << "Erro ao abrir o arquivo de saída!" << endl;
        return;
    }
    
    // Gerar a string DOT do grafo
    string dotString = gerarGrafoDOT(grafo);
    // ajustar os caracteres especiais para que o JSON seja válido
    string dotEscapado = ajustarJSON(dotString);
    
    // Salvar as informações em formato JSON (montagem manual)
    arquivoSaida << "{" << "\n";
    arquivoSaida << "  \"estatisticas\": {" << "\n";
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
    // Supondo que o índice 0 não seja utilizado
    for (size_t i = 1; i < intermediacao.size(); ++i) {
        arquivoSaida << intermediacao[i];
        if (i < intermediacao.size() - 1) {
            arquivoSaida << ", ";
        }
    }
    arquivoSaida << "]\n";
    arquivoSaida << "  },\n";
    arquivoSaida << "  \"grafo_dot\": \"" << dotEscapado << "\"\n";
    arquivoSaida << "}" << "\n";
    
    arquivoSaida.close();
    cout << "\nTodas as informacoes foram salvas em: " << nomeArquivo << endl;
}

// === Função Principal ===

int main() {
	string nomeArquivo;
	
	cout << "Digite o nome do arquivo: ";
	cin >> nomeArquivo;
	while (nomeArquivo != "")
	{
		ifstream arquivo("./instancias/"+nomeArquivo);
		if (!arquivo.is_open()) {
			cerr << "Erro ao abrir o arquivo!" << endl;
			return 1;
		}

		string linha;
		Grafo grafo;
	 
		// Leitura do cabeçalho
		getline(arquivo, linha); 
		grafo.nome = linha.substr(linha.find(":") + 1);
		getline(arquivo, linha); 
		grafo.valorOtimo = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.numVeiculos = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.capacidadeVeiculo = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.deposito = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.numVertices = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.numArestas = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.numArcos = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.numVerticesRequeridos = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.numArestasRequeridas = extrairValorInteiro(linha);
		getline(arquivo, linha); 
		grafo.numArcosRequeridos = extrairValorInteiro(linha);

		getline(arquivo, linha); 
		getline(arquivo, linha);
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

		// Algoritmo de Floyd-Warshall
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

		for (int k = 1; k < N; ++k) {
			for (int i = 1; i < N; ++i) {
				for (int j = 1; j < N; ++j) {
					if (dist[i][k] != INF && dist[k][j] != INF &&
						dist[i][k] + dist[k][j] < dist[i][j]) {
						dist[i][j] = dist[i][k] + dist[k][j];
						pred[i][j] = pred[k][j];
					}
				}
			}
		}

		// Impressão da matriz de caminhos mais curtos (dist)
		cout << "\nMatriz de caminhos mais curtos (dist[i][j]):\n";
		for (int i = 1; i < N; ++i) {
			for (int j = 1; j < N; ++j) {
				if (dist[i][j] == INF) cout << "INF ";
				else cout << dist[i][j] << " ";
			}
			cout << endl;
		}

		// Impressão da matriz de predecessores (pred)
		cout << "\nMatriz de predecessores (pred[i][j]):\n";
		for (int i = 1; i < N; ++i) {
			for (int j = 1; j < N; ++j) {
				cout << pred[i][j] << " ";
			}
			cout << endl;
		}

		// Estatísticas
		double densidade = calcularDensidade(grafo.numVertices, grafo.arestas.size(), grafo.arcos.size());
		int grauMin, grauMax;
		calcularGraus(grafo, grafo.numVertices, grauMin, grauMax);
		int componentes = calcularComponentesConectados(grafo, grafo.numVertices);
		pair<double, int> res = calcularCaminhoMedioDiametro(dist, grafo.numVertices);
		vector<int> intermediacao = calcularIntermediacao(pred, dist, grafo.numVertices);

		// Exibir resultados
		exibirEstatisticasFormatadas(grafo, densidade, componentes, grauMin, grauMax, res.first, res.second, intermediacao);
		
		
		salvarEmArquivo(grafo, densidade, componentes, grauMin, grauMax, res.first, res.second, intermediacao, "estatisticas.json");

	
		cout << endl << endl << "Digite o nome do arquivo: ";
		cin >> nomeArquivo;
	}

    return 0;
}
