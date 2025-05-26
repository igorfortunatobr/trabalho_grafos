#include "grafo.hpp"
#include <vector>
#include <cmath>
#include <limits>
#include <random>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iostream>
#include <map>

using namespace std;

const double INFLUENCIA_FEROMONIO = 1.0; // Influência do feromônio
const double INFLUENCIA_HEURISTICA = 3.0; // Influência da heurística (1/dist)
const double TAXA_EVAPORACAO_FEROMONIO = 0.2; // Taxa de evaporação

const int NUM_FORMIGAS = 10;
const int NUM_ITERACOES = 300;


// Representa um serviço (nó, aresta ou arco requerido)
struct Servico {
    int id;
    int iVertice1, iVertice2; // extremidades
    int demanda;
    int custo;
    bool atendido;
};

// Representa uma rota construída por uma formiga
struct Rota {
    vector<Servico> vsServicos;
    int demandaTotal = 0;
    int custoTotal = 0;
};

// Representa a solução de uma formiga (um conjunto de rotas)
struct Solucao {
    vector<Rota> rotas;
    int iCustoTotal = 0;
};

// Função heurística (quanto menor a distância, melhor)
double heuristica(int distancia) {
    return distancia > 0 ? 1.0 / distancia : 0.0001;
}

// Escolhe o próximo serviço baseado em probabilidade
int escolherProximo(const Servico& atual, const vector<Servico>& candidatos, const map<pair<int, int>, double>& feromonio, const vector<vector<int>>& dist, int deposito) {
    vector<double> vdProbabilidade;
    double soma = 0;

    for (const Servico& s : candidatos) {
        auto chave = make_pair(atual.iVertice1, s.iVertice1);
		double f = pow(feromonio.at(chave), INFLUENCIA_FEROMONIO);

		// Novo cálculo heurístico baseado no custo total de inclusão
		int custoInclusao = dist[atual.iVertice1][s.iVertice1] + s.custo + dist[s.iVertice2][deposito];

		double h = pow(1.0 / (custoInclusao + 1), INFLUENCIA_HEURISTICA);

		vdProbabilidade.push_back(f * h);
        soma += vdProbabilidade.back();
    }

    // Roleta para gerar valor aleatório, conforme o conceito do ACO
    // Cria um gerador de números aleatórios baseado no sistema, usado para inicializar o gerador aleatório com uma semente.
    random_device geradorSeed;
    // Inicializa o gerador com a semente gerada pelo random_device
    mt19937 gen(geradorSeed());
    // Cria uma distribuição uniforme real no intervalo (0.0, soma), ou seja, todos valores no intervalo tem mesma chance
    uniform_real_distribution<> dis(0.0, soma);
    double dNumeroAleatorio = dis(gen);
    
    for (int i = 0; i < (int)vdProbabilidade.size(); ++i) {
        dNumeroAleatorio -= vdProbabilidade[i];
        if (dNumeroAleatorio <= 0) return i;
    }
    return (int)vdProbabilidade.size() - 1;
}

// Inicializa a lista de serviços a partir do grafo
vector<Servico> extrairServicos(const Grafo& grafo) {
    vector<Servico> vsLista;
    int id = 1;
    // vértices
    for (const auto& vertice : grafo.vertices)
        if (vertice.requerServico)
            vsLista.push_back({id++, vertice.id, vertice.id, vertice.demanda, vertice.custoAtendimento, false});

    // arestas (use demanda > 0 como critério)
    for (const auto& aresta : grafo.arestas)
        if (aresta.demanda > 0)
            vsLista.push_back({id++, aresta.origem, aresta.destino, aresta.demanda, aresta.custoAtendimento, false});

    // arcos
    for (const auto& arco : grafo.arcos)
        if (arco.requerServico)  // aqui você já estava marcando corretamente
            vsLista.push_back({id++, arco.origem, arco.destino, arco.demanda, arco.custoAtendimento, false});

    return vsLista;
}

void otimizarRota2opt(Rota& rota, const vector<vector<int>>& distancias, int deposito) {
    if (rota.vsServicos.size() < 3) return; // nada a otimizar

    bool melhorou;
    do {
        melhorou = false;

        for (int i = 0; i < (int)rota.vsServicos.size() - 1; ++i) {
            for (int j = i + 1; j < (int)rota.vsServicos.size(); ++j) {
                // cria uma cópia temporária da rota
                vector<Servico> novaOrdem = rota.vsServicos;

                // inverte o segmento [i, j]
                reverse(novaOrdem.begin() + i, novaOrdem.begin() + j + 1);

                // calcula o novo custo
                int novoCusto = 0;
                int atual = deposito;

                for (const auto& servico : novaOrdem) {
                    novoCusto += distancias[atual][servico.iVertice1];
                    novoCusto += servico.custo;
                    atual = servico.iVertice2;
                }

                novoCusto += distancias[atual][deposito]; // volta ao depósito

                if (novoCusto < rota.custoTotal) {
                    // melhora encontrada
                    rota.vsServicos = novaOrdem;
                    rota.custoTotal = novoCusto;
                    melhorou = true;
                    break;
                }
            }
            if (melhorou) break;
        }

    } while (melhorou);
}


// Construção de solução por uma formiga
Solucao construirSolucao(const Grafo& grafo, const vector<vector<int>>& distancias, map<pair<int, int>, double>& feromonio) {
    vector<Servico> servicos = extrairServicos(grafo);
    Solucao solucao;

    while (any_of(servicos.begin(), servicos.end(), [](Servico& servico){ return !servico.atendido; })) {
        Rota rota;
        int atual = grafo.deposito;
        //rota.custoTotal += distancias[grafo.deposito][atual];

        while (true) {
            vector<Servico> candidatos;
            for (auto& servico : servicos) {
                if (!servico.atendido && rota.demandaTotal + servico.demanda <= grafo.capacidadeVeiculo)
                    candidatos.push_back(servico);
            }
            if (candidatos.empty()) break;

            Servico servicoFicticioBase = {0, atual, atual, 0, 0, true};
            int idEscolhido = escolherProximo(servicoFicticioBase, candidatos, feromonio, distancias, grafo.deposito);
            Servico servicoEscolhido = candidatos[idEscolhido];
            
			for (auto& servico : servicos) {
				if (servico.id == servicoEscolhido.id) {
					servico.atendido = true;
					break;
				}
			}

            rota.custoTotal += distancias[atual][servicoEscolhido.iVertice1] + servicoEscolhido.custo;
            rota.demandaTotal += servicoEscolhido.demanda;
            rota.vsServicos.push_back(servicoEscolhido);

            atual = servicoEscolhido.iVertice2;
        }

        rota.custoTotal += distancias[atual][grafo.deposito];

		// Otimização local da rota com 2-opt
		otimizarRota2opt(rota, distancias, grafo.deposito);

		solucao.iCustoTotal += rota.custoTotal;
		solucao.rotas.push_back(rota);
    }

    return solucao;
}

// Função principal do ACO
Solucao executarACO(const Grafo& grafo, const vector<vector<int>>& dist) {
    int iNumeroVertices = grafo.numVertices + 1;
    map<pair<int, int>, double> feromonio;
    
	for (int i = 0; i < iNumeroVertices; ++i)
		for (int j = 0; j < iNumeroVertices; ++j)
			feromonio[{i, j}] = 1.0;

    Solucao melhorSolucao;
    melhorSolucao.iCustoTotal = INF;

    for (int iteracao = 0; iteracao < NUM_ITERACOES; ++iteracao) {
        vector<Solucao> vsPopulacaoSolucoes;

        for (int formiga = 0; formiga < NUM_FORMIGAS; ++formiga) {
            Solucao solucao = construirSolucao(grafo, dist, feromonio);
            if (solucao.iCustoTotal < melhorSolucao.iCustoTotal) {
                melhorSolucao = solucao;
            }
            vsPopulacaoSolucoes.push_back(solucao);
        }

        // Evaporação
        for (auto& par : feromonio)
			par.second *= (1.0 - TAXA_EVAPORACAO_FEROMONIO);

        // Atualiza feromônio com base nas soluções
        for (const auto& solucao : vsPopulacaoSolucoes) {
            for (const auto& rota : solucao.rotas) {
                int atual = grafo.deposito;
                for (const auto& servico : rota.vsServicos) {
                    feromonio[{atual, servico.iVertice1}] += 1.0 / solucao.iCustoTotal;
                    atual = servico.iVertice2;
                }
                feromonio[{atual, grafo.deposito}] += 1.0 / solucao.iCustoTotal;
            }
        }
        
        // Reforço da melhor solução global (elitismo)
        for (const auto& rota : melhorSolucao.rotas) {
            int atual = grafo.deposito;
            for (const auto& servico : rota.vsServicos) {
                feromonio[{atual, servico.iVertice1}] += 1.0 / melhorSolucao.iCustoTotal;
                atual = servico.iVertice2;
            }
            feromonio[{atual, grafo.deposito}] += 1.0 / melhorSolucao.iCustoTotal;
        }
    }

    return melhorSolucao;
}

// Imprime uma única rota
void imprimirRota(int index, const Rota& rota) {
    cout << "Rota " << index+1
              << " | Custo: " << rota.custoTotal
              << " | Demanda total: " << rota.demandaTotal
              << "\n  Servicos:\n";
    for (const auto& servico : rota.vsServicos) {
        cout << "    - Serviço " << servico.id
			 << " (" << servico.iVertice1 << "->" << servico.iVertice2 << ")"
			 << " | Demanda: " << servico.demanda
			 << " | Custo atendimento: " << servico.custo
			 << endl;
    }
}

// Imprime a solução completa
void imprimirSolucao(const Solucao& sSolucao) {
    cout << "=== Solucao ACO ===" << endl;
    cout << "Custo total da solucao: " << sSolucao.iCustoTotal << endl << endl;
    
    for (int i = 0; i < (int)sSolucao.rotas.size(); ++i) {
        imprimirRota(i, sSolucao.rotas[i]);
        cout << endl;
    }
}

void salvarSolucaoDat(
						 const Solucao& sSolucao,
						 int iDeposito,
						 int iDia,
						 long lClockExecucaoReferencia,
						 long lClockParaAcharSolucaoRef,
						 const string& sNomeArquivo
					 )
{
	string sDiretorioSolucao = "./solucoes/";
    ofstream ofs(sDiretorioSolucao + sNomeArquivo);
    if (!ofs.is_open()) {
        cerr << "Erro ao abrir o arquivo para escrita: " << sNomeArquivo << "\n";
        return;
    }

    // 1) cabeçalho
    ofs << sSolucao.iCustoTotal        << "\n"  // custo total da solução
        << sSolucao.rotas.size()      << "\n"  // total de rotas
        << lClockExecucaoReferencia   << "\n"  // clocks na execução do alg ref
        << lClockParaAcharSolucaoRef  << "\n"; // clocks até achar solução

    // 2) cada rota
    for (int i = 0; i < (int)sSolucao.rotas.size(); ++i) {
        const Rota& rota = sSolucao.rotas[i];
        
        int idRota       = int(i) + 1;
        int visitas      = int(rota.vsServicos.size()) + 2; 
        // +2 porque inclui visita inicial e final ao depósito

        // linha básica: depósito, dia, idRota, demanda total, custo total, visitas
        ofs << " " 
			<< iDeposito  		 << " "
            << iDia       	     << " "
            << idRota    		 << " "
            << rota.demandaTotal << " "
            << rota.custoTotal   << "  "
            << visitas;

        // tripla inicial de depósito
        ofs << " (D " << iDeposito << "," << iDia << "," << iDia << ")";

        // para cada serviço
        for (const auto& servico : rota.vsServicos) {
            ofs << " (S " 
                << servico.id << ","    // identificador do serviço
                << servico.iVertice1  << ","    // extremidade de partida
                << servico.iVertice2  << ")";   // extremidade de chegada
        }

        // tripla final de depósito
        ofs << " (D " << iDeposito << "," << iDia << "," << iDia << ")\n";
    }

    ofs.close();
}
