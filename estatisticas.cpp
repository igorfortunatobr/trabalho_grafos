#include "grafo.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

double dCalcularDensidade(int iNumVertices, int iNumArestas, int iNumArcos, bool bEhDirecionado) {
    if (iNumVertices <= 1) return 0.0;
    return static_cast<double>((2 * iNumArestas) + iNumArcos) / (iNumVertices * (iNumVertices - 1));
}

void calcularGraus(const sGrafo& sGrafo, int iNumVertices, int& iGrauMinimo, int& iGrauMaximo) {
    vector<int> viGraus(iNumVertices + 1, 0);
    for (const sAresta& sAresta : sGrafo.vsArestas) {
        viGraus[sAresta.origem]++;
        viGraus[sAresta.destino]++;
    }
    for (const sArco& sArco : sGrafo.vsArcos) {
        viGraus[sArco.origem]++;
        viGraus[sArco.destino]++;
    }
    iGrauMinimo = INF;
    iGrauMaximo = -INF;
    for (int iIndice = 1; iIndice <= iNumVertices; ++iIndice) {
        iGrauMinimo = min(iGrauMinimo, viGraus[iIndice]);
        iGrauMaximo = max(iGrauMaximo, viGraus[iIndice]);
    }
}

void dfsVisitar(int iVertice, const vector<vector<int>>& vviAdjacencias, vector<bool>& vbVisitados) {
    vbVisitados[iVertice] = true;
    for (int iVizinho : vviAdjacencias[iVertice]) {
        if (!vbVisitados[iVizinho]) dfsVisitar(iVizinho, vviAdjacencias, vbVisitados);
    }
}

int iCalcularComponentesConectados(const sGrafo& sGrafo, int iNumVertices) {
    vector<vector<int>> vviAdjacencias(iNumVertices + 1);
    for (const sAresta& sAresta : sGrafo.vsArestas) {
        vviAdjacencias[sAresta.origem].push_back(sAresta.destino);
        vviAdjacencias[sAresta.destino].push_back(sAresta.origem);
    }
    for (const sArco& sArco : sGrafo.vsArcos) {
        vviAdjacencias[sArco.origem].push_back(sArco.destino);
        vviAdjacencias[sArco.destino].push_back(sArco.origem);
    }
    vector<bool> vbVisitados(iNumVertices + 1, false);
    int iComponentes = 0;
    for (int iIndice = 1; iIndice <= iNumVertices; ++iIndice) {
        if (!vbVisitados[iIndice]) {
            dfsVisitar(iIndice, vviAdjacencias, vbVisitados);
            iComponentes++;
        }
    }
    return iComponentes;
}

pair<double, int> calcularCaminhoMedioDiametro(const vector<vector<int>>& vviDistancias, int iNumVertices) {
    int iTotal = 0, iPares = 0, iDiametro = 0;
    for (int iLinha = 1; iLinha <= iNumVertices; ++iLinha) {
        for (int iColuna = 1; iColuna <= iNumVertices; ++iColuna) {
            if (iLinha != iColuna && vviDistancias[iLinha][iColuna] != INF) {
                iTotal += vviDistancias[iLinha][iColuna];
                iPares++;
                iDiametro = max(iDiametro, vviDistancias[iLinha][iColuna]);
            }
        }
    }
    double dCaminhoMedio = (iPares > 0) ? static_cast<double>(iTotal) / iPares : 0.0;
    return {dCaminhoMedio, iDiametro};
}

vector<int> viCalcularIntermediacoes(const vector<vector<int>>& vviPredecessores, const vector<vector<int>>& vviDistancias, int iNumVertices) {
    vector<int> viIntermediacoes(iNumVertices + 1, 0);
    for (int iOrigem = 1; iOrigem <= iNumVertices; ++iOrigem) {
        for (int iDestino = 1; iDestino <= iNumVertices; ++iDestino) {
            if (iOrigem != iDestino && vviDistancias[iOrigem][iDestino] != INF) {
                int iAtual = iDestino;
                while (vviPredecessores[iOrigem][iAtual] != -1 && vviPredecessores[iOrigem][iAtual] != iOrigem) {
                    viIntermediacoes[vviPredecessores[iOrigem][iAtual]]++;
                    iAtual = vviPredecessores[iOrigem][iAtual];
                }
            }
        }
    }
    return viIntermediacoes;
}

string sAjustarJSON(const string& sJsonEntrada) {
    string sJsonSaida;
    for (char c : sJsonEntrada) {
        if (c == '"') sJsonSaida += "\\\"";
        else if (c == '\\') sJsonSaida += "\\\\";
        else if (c == '\n') sJsonSaida += "\\n";
        else if (c == '\r') sJsonSaida += "\\r";
        else if (c == '\t') sJsonSaida += "\\t";
        else sJsonSaida.push_back(c);
    }
    return sJsonSaida;
}

string sGerarGrafoJSON(const sGrafo &sGrafo) {
    stringstream ss;
    ss << "{\n";
    
    // Lista de vértices
    ss << "\t\"vertices\": [\n";
    for (size_t iIndice = 0; iIndice < sGrafo.vsVertices.size(); iIndice++) {
        ss << "\t\t{ \"id\": " << sGrafo.vsVertices[iIndice].id << " }";
        if (iIndice != sGrafo.vsVertices.size() - 1)
            ss << ",";
        ss << "\n";
    }
    ss << "\t],\n";
    
    // Lista de arestas (caminhos não direcionados)
    ss << "\t\"arestas\": [\n";
    for (size_t iIndice = 0; iIndice < sGrafo.vsArestas.size(); iIndice++) {
        ss << "\t\t{ \"origem\": " << sGrafo.vsArestas[iIndice].origem 
           << ", \"destino\": " << sGrafo.vsArestas[iIndice].destino 
           << ", \"custoTransito\": " << sGrafo.vsArestas[iIndice].custoTransito << " }";
        if (iIndice != sGrafo.vsArestas.size() - 1)
            ss << ",";
        ss << "\n";
    }
    ss << "\t],\n";
    
    // Lista de arcos (caminhos direcionados)
    ss << "\t\"arcos\": [\n";
    for (size_t iIndice = 0; iIndice < sGrafo.vsArcos.size(); iIndice++) {
        ss << "\t\t{ \"origem\": " << sGrafo.vsArcos[iIndice].origem 
           << ", \"destino\": " << sGrafo.vsArcos[iIndice].destino 
           << ", \"custoTransito\": " << sGrafo.vsArcos[iIndice].custoTransito << " }";
        if (iIndice != sGrafo.vsArcos.size() - 1)
            ss << ",";
        ss << "\n";
    }
    ss << "\t]\n";
    ss << "}\n";
    return ss.str();
}

void exibirEstatisticasFormatadas(const sGrafo& sGrafo, double dDensidade, int iComponentes,
    int iGrauMinimo, int iGrauMaximo, double dCaminhoMedio, int iDiametro,
    const vector<int>& viIntermediacoes) {

    cout << "\n=== Estatisticas ===\n";
    cout << "1. Quantidade de vertices: " << sGrafo.iNumVertices << endl;
    cout << "2. Quantidade de arestas: " << sGrafo.vsArestas.size() << endl;
    cout << "3. Quantidade de arcos: " << sGrafo.vsArcos.size() << endl;
    cout << "4. Quantidade de vertices requeridos: " << sGrafo.numVerticesRequeridos << endl;
    cout << "5. Quantidade de arestas requeridas: " << sGrafo.numArestasRequeridas << endl;
    cout << "6. Quantidade de arcos requeridos: " << sGrafo.numArcosRequeridos << endl;
    cout << "7. Densidade do grafo (order strength): " << dDensidade << endl;
    cout << "8. Componentes conectados: " << iComponentes << endl;
    cout << "9. Grau minimo dos vertices: " << iGrauMinimo << endl;
    cout << "10. Grau maximo dos vertices: " << iGrauMaximo << endl;
    cout << "11. Intermediacao:" << endl;
    for (int iIndice = 1; iIndice <= sGrafo.iNumVertices; ++iIndice) {
        cout << "Vertice " << iIndice << ": " << viIntermediacoes[iIndice] << endl;
    }
    cout << "12. Caminho medio: " << dCaminhoMedio << endl;
    cout << "13. Diametro: " << iDiametro << endl;
}

void salvarEmArquivo(const sGrafo& grafo, double dDensidade, int iComponentes,
    int iGrauMinimo, int iGrauMaximo, double dCaminhoMedio, int iDiametro,
    const vector<int>& viIntermediacoes, const string& sNomeArquivo, 
    const vector<vector<int>>& vviDistancias, const vector<vector<int>>& vviPredecessores) {

    ofstream arquivoSaida(sNomeArquivo);
    if (!arquivoSaida.is_open()) {
        cerr << "Erro ao abrir o arquivo de saída!" << endl;
    } else {
        // Gera a string JSON do grafo e a ajusta para ser inclusa no JSON final
        string sGrafoString = sGerarGrafoJSON(grafo);
        string sJsonAjustado = sAjustarJSON(sGrafoString);

        arquivoSaida << "{\n";
        arquivoSaida << "  \"estatisticas\": {\n";
        arquivoSaida << "    \"nome\": \"" << grafo.nome << "\",\n";
        arquivoSaida << "    \"numVertices\": " << grafo.iNumVertices << ",\n";
        arquivoSaida << "    \"numArestas\": " << grafo.vsArestas.size() << ",\n";
        arquivoSaida << "    \"numArcos\": " << grafo.vsArcos.size() << ",\n";
        arquivoSaida << "    \"numVerticesRequeridos\": " << grafo.numVerticesRequeridos << ",\n";
        arquivoSaida << "    \"numArestasRequeridas\": " << grafo.numArestasRequeridas << ",\n";
        arquivoSaida << "    \"numArcosRequeridos\": " << grafo.numArcosRequeridos << ",\n";
        arquivoSaida << "    \"densidade\": " << dDensidade << ",\n";
        arquivoSaida << "    \"componentes\": " << iComponentes << ",\n";
        arquivoSaida << "    \"grauMin\": " << iGrauMinimo << ",\n";
        arquivoSaida << "    \"grauMax\": " << iGrauMaximo << ",\n";
        arquivoSaida << "    \"caminhoMedio\": " << dCaminhoMedio << ",\n";
        arquivoSaida << "    \"diametro\": " << iDiametro << ",\n";
        arquivoSaida << "    \"intermediacao\": [";
        for (size_t i = 0; i < viIntermediacoes.size(); ++i) {
            arquivoSaida << viIntermediacoes[i];
            if (i < viIntermediacoes.size() - 1) 
                arquivoSaida << ", ";
        }
        arquivoSaida << "]\n";
        
        arquivoSaida << "   },\n";

        // Inserção da Matriz de Distância
        arquivoSaida << "    \"matrizDistancia\": [\n";
        for (int i = 1; i <= grafo.iNumVertices; ++i) {
            arquivoSaida << "      [";
            for (int j = 1; j <= grafo.iNumVertices; ++j) {
                if (vviDistancias[i][j] == INF)
                    arquivoSaida << "\"INF\"";
                else
                    arquivoSaida << vviDistancias[i][j];
                if (j <= grafo.iNumVertices - 1)
                    arquivoSaida << ", ";
            }
            arquivoSaida << "]";
            if (i <= grafo.iNumVertices - 1)
                arquivoSaida << ",\n";
            else
                arquivoSaida << "\n";
        }
        arquivoSaida << "    ],\n";

        // Inserção da Matriz de Predecessores
        arquivoSaida << "    \"matrizPredecessores\": [\n";
        for (int i = 1; i <= grafo.iNumVertices; ++i) {
            arquivoSaida << "      [";
            for (int j = 1; j <= grafo.iNumVertices; ++j) {
                if (vviPredecessores[i][j] == -1)
                    arquivoSaida << "\"NULO\"";
                else
                    arquivoSaida << vviPredecessores[i][j];
                if (j <= grafo.iNumVertices - 1)
                    arquivoSaida << ", ";
            }
            arquivoSaida << "]";
            if (i <= grafo.iNumVertices - 1)
                arquivoSaida << ",\n";
            else
                arquivoSaida << "\n";
        }
        arquivoSaida << "    ],\n";

        arquivoSaida << "  \"grafo_json\": \"" << sJsonAjustado << "\"\n";
        arquivoSaida << "}\n";

        arquivoSaida.close();
        cout << "\nInformacoes salvas em: " << sNomeArquivo << endl;
    }
}
