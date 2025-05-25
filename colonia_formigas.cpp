#include "grafo.hpp"
#include <vector>
#include <cmath>
#include <limits>
#include <random>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iostream>
using namespace std;

const double ALFA = 1.0; // Influência do feromônio
const double BETA = 2.0; // Influência da heurística (1/dist)
const double RHO = 0.5; // Taxa de evaporação
const int NUM_FORMIGAS = 10;
const int NUM_ITERACOES = 100;


// Representa um serviço (nó, aresta ou arco requerido)
struct Servico {
    int id;
    int u, v; // extremidades
    int demanda;
    int custo;
    bool atendido;
};

// Representa uma rota construída por uma formiga
struct Rota {
    vector<Servico> servicos;
    int demandaTotal = 0;
    int custoTotal = 0;
};

// Representa a solução de uma formiga (um conjunto de rotas)
struct Solucao {
    vector<Rota> rotas;
    int custoTotal = 0;
};

// Função heurística (quanto menor a distância, melhor)
double heuristica(int dist) {
    return dist > 0 ? 1.0 / dist : 0.0001;
}

// Escolhe o próximo serviço baseado em probabilidade
int escolherProximo(const Servico& atual, const vector<Servico>& candidatos, const vector<vector<double>>& feromonio, const vector<vector<int>>& dist) {
    vector<double> prob;
    double soma = 0;

    for (const Servico& s : candidatos) {
        double f = pow(feromonio[atual.u][s.u], ALFA);
        double h = pow(heuristica(dist[atual.u][s.u]), BETA);
        prob.push_back(f * h);
        soma += prob.back();
    }

    // Roleta
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, soma);
    double r = dis(gen);
    
    for (size_t i = 0; i < prob.size(); ++i) {
        r -= prob[i];
        if (r <= 0) return i;
    }
    return (int)prob.size() - 1;
}

// Inicializa a lista de serviços a partir do grafo
vector<Servico> extrairServicos(const Grafo& grafo) {
    vector<Servico> lista;
    int id = 1;
    // vértices
    for (const auto& v : grafo.vertices)
        if (v.requerServico)
            lista.push_back({id++, v.id, v.id, v.demanda, v.custoAtendimento, false});

    // arestas (use demanda > 0 como critério)
    for (const auto& e : grafo.arestas)
        if (e.demanda > 0)
            lista.push_back({id++, e.origem, e.destino, e.demanda, e.custoAtendimento, false});

    // arcos
    for (const auto& a : grafo.arcos)
        if (a.requerServico)  // aqui você já estava marcando corretamente
            lista.push_back({id++, a.origem, a.destino, a.demanda, a.custoAtendimento, false});

    return lista;
}

// Exemplo simples de twoOpt:
Rota twoOpt(const Rota& r, const vector<vector<int>>& dist) {
    Rota best = r;
    bool melhorou = true;
    while (melhorou) {
        melhorou = false;
        for (int i = 1; i + 2 < (int)best.servicos.size(); ++i) {
            for (int j = i + 1; j + 1 < (int)best.servicos.size(); ++j) {
                // calcula ganho de trocar trecho [i..j]
                int antes = dist[best.servicos[i-1].u][best.servicos[i].u]
                          + dist[best.servicos[j].u][best.servicos[j+1].u];
                int depois = dist[best.servicos[i-1].u][best.servicos[j].u]
                           + dist[best.servicos[i].u][best.servicos[j+1].u];
                if (depois + 0 < antes) {
                    reverse(best.servicos.begin()+i, best.servicos.begin()+j+1);
                    melhorou = true;
                }
                if (melhorou) break;
            }
            if (melhorou) break;
        }
    }
    // Recalcule custoTotal
    best.custoTotal = 0;
    for (int k = 0; k + 1 < (int)best.servicos.size(); ++k)
        best.custoTotal += dist[best.servicos[k].u][best.servicos[k+1].u]
                         + best.servicos[k+1].custo;
    return best;
}

// Construção de solução por uma formiga
Solucao construirSolucao(const Grafo& grafo, const vector<vector<int>>& dist, vector<vector<double>>& feromonio) {
    vector<Servico> servicos = extrairServicos(grafo);
    Solucao solucao;

    while (any_of(servicos.begin(), servicos.end(), [](Servico& s){ return !s.atendido; })) {
        Rota rota;
        int atual = grafo.deposito;
        rota.custoTotal += dist[grafo.deposito][atual];

        while (true) {
            vector<Servico> candidatos;
            for (auto& s : servicos) {
                if (!s.atendido && rota.demandaTotal + s.demanda <= grafo.capacidadeVeiculo)
                    candidatos.push_back(s);
            }
            if (candidatos.empty()) break;

            Servico dummy = {0, atual, atual, 0, 0, true};
            int idx = escolherProximo(dummy, candidatos, feromonio, dist);
            Servico escolhido = candidatos[idx];

            rota.custoTotal += dist[atual][escolhido.u] + escolhido.custo;
            rota.demandaTotal += escolhido.demanda;
            rota.servicos.push_back(escolhido);
            
            rota = twoOpt(rota, dist);

            for (auto& s : servicos) {
                if (s.id == escolhido.id) {
                    s.atendido = true;
                    break;
                }
            }
            atual = escolhido.v;
        }

        rota.custoTotal += dist[atual][grafo.deposito];
        solucao.custoTotal += rota.custoTotal;
        solucao.rotas.push_back(rota);
    }

    return solucao;
}

// Função principal do ACO
Solucao executarACO(const Grafo& grafo, const vector<vector<int>>& dist) {
    int V = grafo.numVertices + 1;
    vector<vector<double>> feromonio(V, vector<double>(V, 1.0));

    Solucao melhorSolucao;
    melhorSolucao.custoTotal = INF;

    for (int it = 0; it < NUM_ITERACOES; ++it) {
        vector<Solucao> populacao;

        for (int f = 0; f < NUM_FORMIGAS; ++f) {
            Solucao solucao = construirSolucao(grafo, dist, feromonio);
            if (solucao.custoTotal < melhorSolucao.custoTotal) {
                melhorSolucao = solucao;
            }
            populacao.push_back(solucao);
        }

        // Evaporação
        for (int i = 0; i < V; ++i)
            for (int j = 0; j < V; ++j)
                feromonio[i][j] *= (1 - RHO);

        // Atualiza feromônio com base nas soluções
        for (const auto& sol : populacao) {
            for (const auto& rota : sol.rotas) {
                int atual = grafo.deposito;
                for (const auto& s : rota.servicos) {
                    feromonio[atual][s.u] += 1.0 / sol.custoTotal;
                    atual = s.v;
                }
                feromonio[atual][grafo.deposito] += 1.0 / sol.custoTotal;
            }
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
    for (const auto& s : rota.servicos) {
        cout << "    - Serviço " << s.id
                  << " (" << s.u << "->" << s.v << ")"
                  << " | Demanda: " << s.demanda
                  << " | Custo atendimento: " << s.custo
                  << "\n";
    }
}

// Imprime a solução completa
void imprimirSolucao(const Solucao& sol) {
    cout << "=== Solucao ACO ===\n";
    cout << "Custo total da solucao: " << sol.custoTotal << "\n\n";
    for (size_t i = 0; i < sol.rotas.size(); ++i) {
        imprimirRota(i, sol.rotas[i]);
        cout << "\n";
    }
}

void salvarSolucaoDat(const Solucao& sol,
                      int deposito,
                      int dia,
                      long clkExecRef,
                      long clkFindSolRef,
                      const string& filename)
{
	string sDiretorioSolucao = "./solucoes/";
    ofstream ofs(sDiretorioSolucao + filename);
    if (!ofs.is_open()) {
        cerr << "Erro ao abrir o arquivo para escrita: " << filename << "\n";
        return;
    }

    // 1) cabeçalho
    ofs << sol.custoTotal        << "\n"  // custo total da solução
        << sol.rotas.size()      << "\n"  // total de rotas
        << clkExecRef            << "\n"  // clocks na execução do alg ref
        << clkFindSolRef         << "\n"; // clocks até achar solução

    // 2) cada rota
    for (size_t i = 0; i < sol.rotas.size(); ++i) {
        const Rota& rota = sol.rotas[i];
        int idRota       = int(i) + 1;
        int visitas      = int(rota.servicos.size()) + 2; 
        // +2 porque inclui visita inicial e final ao depósito

        // linha básica: depósito, dia, idRota, demanda total, custo total, visitas
        ofs << " " << deposito << " "
            << dia       << " "
            << idRota    << " "
            << rota.demandaTotal << " "
            << rota.custoTotal   << "  "
            << visitas;

        // tripla inicial de depósito
        ofs << " (D " << deposito << "," << dia << "," << dia << ")";

        // para cada serviço
        for (const auto& s : rota.servicos) {
            ofs << " (S " 
                << s.id << ","    // identificador do serviço
                << s.u  << ","    // extremidade de partida
                << s.v  << ")";   // extremidade de chegada
        }

        // tripla final de depósito
        ofs << " (D " << deposito << "," << dia << "," << dia << ")\n";
    }

    ofs.close();
}
