#include "grafo.hpp"
#include <iostream>
#include <vector>
#include <string>
#include "leitura.cpp"
#include "estatisticas.cpp"
#include "colonia_formigas.cpp"
#include "arquivosInstancia.hpp"

using namespace std;

void exibirMatrizDistancias(const vector<vector<int>>& vviDistancias, int iNumVertices) {
    cout << "\nMatriz de distâncias (vviDistancias[i][j]):\n";
    for (int iLinha = 1; iLinha <= iNumVertices; ++iLinha) {
        for (int iColuna = 1; iColuna <= iNumVertices; ++iColuna) {
            if (vviDistancias[iLinha][iColuna] == INF)
                cout << "INF\t";
            else
                cout << vviDistancias[iLinha][iColuna] << "\t";
        }
        cout << endl;
    }
}

void exibirMatrizPredecessores(const vector<vector<int>>& vviPredecessores, int iNumVertices) {
    cout << "\nMatriz de predecessores (vviPredecessores[i][j]):\n";
    for (int iLinha = 1; iLinha <= iNumVertices; ++iLinha) {
        for (int iColuna = 1; iColuna <= iNumVertices; ++iColuna) {
            if (vviPredecessores[iLinha][iColuna] == -1)
                cout << "NULO\t";
            else
                cout << vviPredecessores[iLinha][iColuna] << "\t";
        }
        cout << endl;
    }
}

void floydWarshall(sGrafo& Grafo, int iTamanho, vector<vector<int>>& vviDistancias, vector<vector<int>>& vviPredecessores) {
    for (int i = 1; i < iTamanho; ++i) {
        vviDistancias[i][i] = 0;
        vviPredecessores[i][i] = i;
    }
    for (const sAresta& Aresta : Grafo.vsArestas) {
        vviDistancias[Aresta.origem][Aresta.destino] = Aresta.custoTransito;
        vviDistancias[Aresta.destino][Aresta.origem] = Aresta.custoTransito;
        vviPredecessores[Aresta.origem][Aresta.destino] = Aresta.origem;
        vviPredecessores[Aresta.destino][Aresta.origem] = Aresta.destino;
    }
    for (const sArco& Arco : Grafo.vsArcos) {
        vviDistancias[Arco.origem][Arco.destino] = Arco.custoTransito;
        vviPredecessores[Arco.origem][Arco.destino] = Arco.origem;
    }
    for (int k = 1; k < iTamanho; ++k) {
        for (int i = 1; i < iTamanho; ++i) {
            for (int j = 1; j < iTamanho; ++j) {
                if (vviDistancias[i][k] != INF && vviDistancias[k][j] != INF && vviDistancias[i][k] + vviDistancias[k][j] < vviDistancias[i][j]) {
                    vviDistancias[i][j] = vviDistancias[i][k] + vviDistancias[k][j];
                    vviPredecessores[i][j] = vviPredecessores[k][j];
                }
            }
        }
    }
}

int main(int iQtdArgumentos, char* vsArgumentos[]) {
    int iNumFormigas = 20;
    int iNumIteracoes = 300;
    double dInfluenciaFeromonio = 1.0;
    double dInfluenciaHeuristica = 3.0;

    if (iQtdArgumentos > 1) iNumFormigas = atoi(vsArgumentos[1]);
    if (iQtdArgumentos > 2) iNumIteracoes = atoi(vsArgumentos[2]);
    if (iQtdArgumentos > 3) dInfluenciaFeromonio = atof(vsArgumentos[3]);
    if (iQtdArgumentos > 4) dInfluenciaHeuristica = atof(vsArgumentos[4]);

    string sNomeArquivoInstancia;
    int iQtdArquivos = (int)vsArquivosTrabalho.size();
    cout << "Qtd. Arquivos: " << iQtdArquivos << endl;

    for (int iIndiceArquivo = 0; iIndiceArquivo < iQtdArquivos; iIndiceArquivo++) {
        clock_t tTempoInicio = clock();
        sNomeArquivoInstancia = vsArquivosTrabalho[iIndiceArquivo];
        cout << "Processando arquivo " << sNomeArquivoInstancia << endl;
        bool bConseguiuLerArquivo = false;
        
        sGrafo Grafo = sLerArquivo(sNomeArquivoInstancia, bConseguiuLerArquivo);
        
        if (bConseguiuLerArquivo) {
			
            bool bEhDirecionado = !Grafo.vsArcos.empty();
            int iTamanho = Grafo.iNumVertices + 1;
            
            vector<vector<int>> vviDistancias(iTamanho, vector<int>(iTamanho, INF));
            vector<vector<int>> vviPredecessores(iTamanho, vector<int>(iTamanho, -1));
            floydWarshall(Grafo, iTamanho, vviDistancias, vviPredecessores);
            
           // exibirMatrizDistancias(vviDistancias, Grafo.iNumVertices);
            
           // exibirMatrizPredecessores(vviPredecessores, Grafo.iNumVertices);
            
            double dDensidade = dCalcularDensidade(Grafo.iNumVertices, Grafo.vsArestas.size(), Grafo.vsArcos.size(), bEhDirecionado);
            
            int iGrauMinimo, iGrauMaximo;
            calcularGraus(Grafo, Grafo.iNumVertices, iGrauMinimo, iGrauMaximo);
            
            int iComponentesConectados = iCalcularComponentesConectados(Grafo, Grafo.iNumVertices);
            pair<double, int> parCaminhoMedioDiametro = calcularCaminhoMedioDiametro(vviDistancias, Grafo.iNumVertices);
            vector<int> viIntermediacoes = viCalcularIntermediacoes(vviPredecessores, vviDistancias, Grafo.iNumVertices);
            
            cout << "Gerando arquivo de estatisticas" << endl;
            exibirEstatisticasFormatadas(Grafo, dDensidade, iComponentesConectados, iGrauMinimo, iGrauMaximo, parCaminhoMedioDiametro.first, parCaminhoMedioDiametro.second, viIntermediacoes);
            string sNomeArquivoEstatisticas = "estatisticas/estatisticas_" + Grafo.nome + ".json";
            salvarEmArquivo(Grafo, dDensidade, iComponentesConectados, iGrauMinimo, iGrauMaximo, parCaminhoMedioDiametro.first, parCaminhoMedioDiametro.second, viIntermediacoes, sNomeArquivoEstatisticas, vviDistancias, vviPredecessores);
            cout << "Executando solucao inicial" << endl;
            sSolucao Solucao = executarACO(Grafo, vviDistancias, iNumFormigas, iNumIteracoes, dInfluenciaFeromonio, dInfluenciaHeuristica);
            buscaLocal(Solucao, vviDistancias, Grafo.deposito, Grafo.capacidadeVeiculo);
            clock_t tTempoFim = clock();
            long lClockParaAcharSolucao = tTempoFim - tTempoInicio;
            string sNomeArquivoSolucao = "sol-" + sNomeArquivoInstancia;
            cout << "Salvando solucao" << endl;
            salvarSolucaoDat(Solucao, Grafo.deposito, 1, tTempoFim, lClockParaAcharSolucao, sNomeArquivoSolucao);
            cout << "Solucao salva em " + sNomeArquivoSolucao << endl;
        } else {
            cerr << "Erro ao abrir o arquivo!" << endl;
        }
        cout << endl;
    }
    return 0;
}
