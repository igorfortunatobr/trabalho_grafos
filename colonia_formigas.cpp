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

double INFLUENCIA_FEROMONIO = 1.0; // Influência do feromônio
double INFLUENCIA_HEURISTICA = 3.0; // Influência da heurística (1/dist)
const double TAXA_EVAPORACAO_FEROMONIO = 0.2; // Taxa de evaporacao

int NUM_FORMIGAS = 20;
int NUM_ITERACOES = 300;


// Representa um servico (no, aresta ou arco requerido)
struct sServico {
    int id;
    int iVertice1, iVertice2; // extremidades
    int demanda;
    int custo;
    bool atendido;
};

// Representa uma rota construída por uma formiga
struct sRota {
    vector<sServico> vsServicos;
    int demandaTotal = 0;
    int custoTotal = 0;
};

// Representa a solucao de uma formiga (um conjunto de rotas)
struct sSolucao {
    vector<sRota> rotas;
    int iCustoTotal = 0;
};

// Funcao heurística (quanto menor a distância, melhor)
double heuristica(int distancia) {
    return distancia > 0 ? 1.0 / distancia : 0.0001;
}

// Escolhe o proximo servico baseado em probabilidade
int escolherProximo(const sServico& atual, const vector<sServico>& candidatos, const map<pair<int, int>, double>& feromonio, const vector<vector<int>>& dist, int deposito) {
    vector<double> vdProbabilidade;
    double soma = 0;

    for (const sServico& s : candidatos) {
        auto chave = make_pair(atual.iVertice1, s.iVertice1);
		double f = pow(feromonio.at(chave), INFLUENCIA_FEROMONIO);

		// Novo cálculo heurístico baseado no custo total de inclusao
		int custoInclusao = dist[atual.iVertice1][s.iVertice1] + s.custo + dist[s.iVertice2][deposito];

		double h = pow(1.0 / (custoInclusao + 1), INFLUENCIA_HEURISTICA);

		vdProbabilidade.push_back(f * h);
        soma += vdProbabilidade.back();
    }

    // Roleta para gerar valor aleatorio, conforme o conceito do ACO
    // Cria um gerador de números aleatorios baseado no sistema, usado para inicializar o gerador aleatorio com uma semente.
    random_device geradorSeed;
    // Inicializa o gerador com a semente gerada pelo random_device
    mt19937 gen(geradorSeed());
    // Cria uma distribuicao uniforme real no intervalo (0.0, soma), ou seja, todos valores no intervalo tem mesma chance
    uniform_real_distribution<> dis(0.0, soma);
    double dNumeroAleatorio = dis(gen);
    
    for (int i = 0; i < (int)vdProbabilidade.size(); ++i) {
        dNumeroAleatorio -= vdProbabilidade[i];
        if (dNumeroAleatorio <= 0) return i;
    }
    return (int)vdProbabilidade.size() - 1;
}

// Inicializa a lista de servicos a partir do grafo
vector<sServico> extrairServicos(const sGrafo& grafo) {
    vector<sServico> vsLista;
    int id = 1;
    // vértices
    for (const auto& vertice : grafo.vsVertices)
        if (vertice.requerServico)
        {
            sServico sNovoServico;
            sNovoServico.id = id++;
            sNovoServico.iVertice1 = vertice.id;
            sNovoServico.iVertice2 = vertice.id;
            sNovoServico.demanda = vertice.demanda;
            sNovoServico.custo = vertice.custoAtendimento;
            sNovoServico.atendido = false;
            vsLista.push_back(sNovoServico);
        }

    // arestas (use demanda > 0 como critério)
    for (const auto& aresta : grafo.vsArestas)
        if (aresta.demanda > 0)
            vsLista.push_back({id++, aresta.origem, aresta.destino, aresta.demanda, aresta.custoAtendimento, false});

    // arcos
    for (const auto& arco : grafo.vsArcos)
        if (arco.requerServico)  // aqui você já estava marcando corretamente
            vsLista.push_back({id++, arco.origem, arco.destino, arco.demanda, arco.custoAtendimento, false});

    return vsLista;
}

// Calcula o custo de uma rota (sequência de servicos) a partir do deposito:
// custoRota corrigido: soma arcos so uma vez no fim
double custoRota(
					const vector<sServico>& seq,
					const vector<vector<int>>& dist,
					int deposito
) {
    double custo = 0.0;
    int atual = deposito;

    for (const auto& s : seq) {
        // ir de 'atual' até o início do servico
        custo += dist[atual][s.iVertice1];
        // custo interno do servico
        custo += s.custo;
        // agora a formiga sai do servico no vértice2
        atual = s.iVertice2;
    }
    // so aqui soma o retorno ao deposito, **uma vez**
    custo += dist[atual][deposito];

    return custo;
}


// 1) 2-opt: tenta inverter todos os trechos [i..j] e aceita se melhorar
bool twoOptTrecho(
					vector<sServico>& vsSequencia,
					const vector<vector<int>>& vviDistancias,
					int iDeposito
) 
{
    int iQuantidadeServicos = vsSequencia.size();
    double custoOriginal = custoRota(vsSequencia, vviDistancias, iDeposito);
    double dMelhorGanho = 0.0;
    int iMelhorI = -1;
    int iMelhorJ = -1;

    for (int i = 0; i < iQuantidadeServicos - 1; ++i) {
        for (int j = i + 1; j < iQuantidadeServicos; ++j) {
            reverse(vsSequencia.begin()+i, vsSequencia.begin()+j+1);
            double novoCusto = custoRota(vsSequencia, vviDistancias, iDeposito);
            double dGanho = custoOriginal - novoCusto;
            
            if (dGanho > dMelhorGanho) {
                dMelhorGanho = dGanho;
                iMelhorI = i; 
                iMelhorJ = j;
            }
            
            reverse(
						vsSequencia.begin() + i, 
						vsSequencia.begin() + j + 1
			);
        }
    }
    
    // Só aceita a troca se a melhoria for significativa 
	// (maior que 0.000001) para evitar aceitar diferenças 
	// causadas por imprecisão numérica 
    if (dMelhorGanho > 1e-6) {
        reverse(
					vsSequencia.begin() + iMelhorI, 
					vsSequencia.begin() + iMelhorJ + 1
		);
        return true;
    }
    
    return false;
}


// 2) Swap simples de dois servicos
bool swapServico(
					vector<sServico>& vsSequencia,
					const vector<vector<int>>& vviDistancias,
					int iDeposito
) 
{
    int iQuantidadeServicos = vsSequencia.size();
    double dMelhorGanho = 0;
    int iMelhorI = -1;
    int iMelhorJ = -1;
    double custoOriginal = custoRota(vsSequencia, vviDistancias, iDeposito);

    for (int i = 0; i < iQuantidadeServicos - 1; ++i) {
        for (int j = i + 1; j < iQuantidadeServicos; ++j) {
            swap(vsSequencia[i], vsSequencia[j]);
            double novoCusto = custoRota(vsSequencia, vviDistancias, iDeposito);
            double dGanho = custoOriginal - novoCusto;
            
            if (dGanho > dMelhorGanho) {
                dMelhorGanho = dGanho;
                iMelhorI = i; 
                iMelhorJ = j;
            }
            
            swap(vsSequencia[i], vsSequencia[j]);
        }
    }

	// Só aceita a troca se a melhoria for significativa 
	// (maior que 0.000001) para evitar aceitar diferenças 
	// causadas por imprecisão numérica 
    if (dMelhorGanho > 1e-6) {
        swap(vsSequencia[iMelhorI], vsSequencia[iMelhorJ]);
        return true;
    }
    
    return false;
}

// 3) Loop principal de busca local: aplica until no move possível
void buscaLocal(
					sSolucao& solucao,
					const vector<vector<int>>& vviDistancias,
					int iDeposito,
					int iCapacidadeVeiculo
) 
{
					
    // Busca local intra-rota (já existente)
    for (auto& rota : solucao.rotas) {
        auto& sequencia = rota.vsServicos;
        bool bMelhorou = true;
        while (bMelhorou) {
            bMelhorou = false;
            if (twoOptTrecho(sequencia, vviDistancias, iDeposito)) { 
				bMelhorou = true; 
				continue; 
			}
            if (swapServico(sequencia, vviDistancias, iDeposito))  { 
				bMelhorou = true; 
			}
        }
        rota.custoTotal = custoRota(sequencia, vviDistancias, iDeposito);
    }
    
    // Busca local entre rotas (Relocate e Exchange)
    bool bMelhoraGlobal = true;
    while (bMelhoraGlobal) {
        bMelhoraGlobal = false;
        // Relocate: move um servico de uma rota para outra
        for (int i = 0; i < (int)solucao.rotas.size(); ++i) {
            for (int j = 0; j < (int)solucao.rotas.size(); ++j) {
                if (i == j) 
					continue;
					
                auto& rotaOriginal = solucao.rotas[i];
                auto& rotaDestino = solucao.rotas[j];
                for (int si = 0; si < (int)rotaOriginal.vsServicos.size(); ++si) {
                    sServico servico = rotaOriginal.vsServicos[si];
                    // So move se couber na rota destino
                    if (rotaDestino.demandaTotal + servico.demanda > iCapacidadeVeiculo) continue;
                    for (int pos = 0; pos <= (int)rotaDestino.vsServicos.size(); ++pos) {
                        // Testa insercao
                        auto sequenciaOriginal = rotaOriginal.vsServicos;
                        auto sequenciaDestino = rotaDestino.vsServicos;
                        sequenciaOriginal.erase(sequenciaOriginal.begin() + si);
                        sequenciaDestino.insert(sequenciaDestino.begin() + pos, servico);
                        double novoCustoOrigem = custoRota(sequenciaDestino, vviDistancias, iDeposito);
                        double novoCustoDestino = custoRota(sequenciaDestino, vviDistancias, iDeposito);
                        double dGanho = (rotaOriginal.custoTotal + rotaDestino.custoTotal) - (novoCustoOrigem + novoCustoDestino);
                        
                        // Só aceita a troca se a melhoria for significativa 
                        // (maior que 0.000001) para evitar aceitar diferenças 
                        // causadas por imprecisão numérica 
                        if (dGanho > 1e-6) {
                            // Aplica movimento
                            rotaOriginal.vsServicos = sequenciaOriginal;
                            rotaDestino.vsServicos = sequenciaDestino;
                            rotaOriginal.demandaTotal -= servico.demanda;
                            rotaDestino.demandaTotal += servico.demanda;
                            rotaOriginal.custoTotal = novoCustoOrigem;
                            rotaDestino.custoTotal = novoCustoDestino;
                            bMelhoraGlobal = true;
                            break;
                        }
                    }
                    if (bMelhoraGlobal) 
						break;
                }
                if (bMelhoraGlobal) 
					break;
            }
            if (bMelhoraGlobal) 
				break;
        }
        
        if (bMelhoraGlobal) 
			continue;
        
        // Exchange: troca servicos entre rotas
        for (int i = 0; i < (int)solucao.rotas.size(); ++i) {
            for (int j = i + 1; j < (int)solucao.rotas.size(); ++j) {
                auto& rotaA = solucao.rotas[i];
                auto& rotaB = solucao.rotas[j];
                for (int iContadorA = 0; iContadorA < (int)rotaA.vsServicos.size(); ++iContadorA) {
                    for (int iContadorB = 0; iContadorB < (int)rotaB.vsServicos.size(); ++iContadorB) {
                        sServico servicoA = rotaA.vsServicos[iContadorA];
                        sServico servicoB = rotaB.vsServicos[iContadorB];
                        int novaDemA = rotaA.demandaTotal - servicoA.demanda + servicoB.demanda;
                        int novaDemB = rotaB.demandaTotal - servicoB.demanda + servicoA.demanda;
                        if (novaDemA > iCapacidadeVeiculo || novaDemB > iCapacidadeVeiculo) continue;
                        auto seqA = rotaA.vsServicos;
                        auto seqB = rotaB.vsServicos;
                        seqA[iContadorA] = servicoB;
                        seqB[iContadorB] = servicoA;
                        double novoCustoA = custoRota(seqA, vviDistancias, iDeposito);
                        double novoCustoB = custoRota(seqB, vviDistancias, iDeposito);
                        double dGanho = (rotaA.custoTotal + rotaB.custoTotal) - (novoCustoA + novoCustoB);
                        
                        // Só aceita a troca se a melhoria for significativa 
                        // (maior que 0.000001) para evitar aceitar diferenças 
                        // causadas por imprecisão numérica 
                        if (dGanho > 1e-6) {
                            rotaA.vsServicos = seqA;
                            rotaB.vsServicos = seqB;
                            rotaA.demandaTotal = novaDemA;
                            rotaB.demandaTotal = novaDemB;
                            rotaA.custoTotal = novoCustoA;
                            rotaB.custoTotal = novoCustoB;
                            bMelhoraGlobal = true;
                            break;
                        }
                    }
                    if (bMelhoraGlobal) break;
                }
                if (bMelhoraGlobal) break;
            }
            if (bMelhoraGlobal) break;
        }
    }
    
    // Atualiza custo total da solucao
    solucao.iCustoTotal = 0;
    for (auto& rota : solucao.rotas)
        solucao.iCustoTotal += rota.custoTotal;
}


// Construcao de solucao por uma formiga
sSolucao construirSolucao(
							const sGrafo& grafo, 
							const vector<vector<int>>& vviDistancias, 
							map<pair<int, int>, double>& dFeromonio
)  {
     // 1) extrai servicos e inicializa flags
    vector<sServico> servicos = extrairServicos(grafo);
    int iTamanho = (int)servicos.size();
    vector<bool> vbAtendido(iTamanho, false);

    // 2) Aqui: gen e dis sao criados UMA ÚNICA VEZ,
    //    antes do laco principal
    mt19937 gen(random_device{}());
    uniform_real_distribution<double> dis(0.0, 1.0);

    sSolucao solucao;
    // <-- Comeca o loop que monta cada rota:
    while (any_of(servicos.begin(), servicos.end(),
                  [](const sServico& servico){ return !servico.atendido; })) {
        sRota rota;
        int iAtual = grafo.deposito;

        // 3) Selecao de candidatos
        while (true) {
            vector<sServico> vsCandidatos;
            for (auto& servico : servicos) {
                if (
						!servico.atendido
						&& rota.demandaTotal + servico.demanda <= grafo.capacidadeVeiculo
				)
                    vsCandidatos.push_back(servico);
            }
            
            if (vsCandidatos.empty()) 
				break;

            // cálculo de probabilidades…
            vector<double> vdProbabilidades(vsCandidatos.size());
            double dSoma = 0;
            for (int i = 0; i < (int)vsCandidatos.size(); ++i) {
                auto chave = make_pair(iAtual, vsCandidatos[i].iVertice1);
                double f = pow(dFeromonio.at(chave), INFLUENCIA_FEROMONIO);
                // custo mínimo de ir até o servico
			    int iCustoIda = vviDistancias[iAtual][vsCandidatos[i].iVertice1];
			    // custo mínimo de voltar ao deposito apos o servico
			    int iCustoVolta = vviDistancias[vsCandidatos[i].iVertice2][grafo.deposito];
			    int iCustoTotal = iCustoIda + 
								  vsCandidatos[i].custo + 
								  iCustoVolta;
								
                double h = pow(1.0 / (iCustoTotal + 1), INFLUENCIA_HEURISTICA);
				vdProbabilidades[i] = f * h;
                dSoma += vdProbabilidades[i];
            }

            // roleta usando o **mesmo** gen e dis
            double dRoleta = dis(gen) * dSoma;
            double dCumulativo = 0;
            
            int idEscolhido = (int)vdProbabilidades.size() - 1;
            
            for (int i = 0; i < (int)vdProbabilidades.size(); ++i) {
                dCumulativo += vdProbabilidades[i];
                
                if (dRoleta <= dCumulativo) { 
					idEscolhido = (int)i; 
					break; 
				}
            }

            // marca e avanca…
            sServico& servicoSelecionado = vsCandidatos[idEscolhido];
            for (auto& servico : servicos)
                if (servico.id == servicoSelecionado.id) { 
					servico.atendido = true; 
					break; 
				}
            rota.vsServicos.push_back(servicoSelecionado);
            rota.demandaTotal += servicoSelecionado.demanda;
            rota.custoTotal   += vviDistancias[iAtual][servicoSelecionado.iVertice1] + servicoSelecionado.custo;
            iAtual = servicoSelecionado.iVertice2;
        }

        rota.custoTotal += vviDistancias[iAtual][grafo.deposito];
        solucao.iCustoTotal += rota.custoTotal;
        solucao.rotas.push_back(rota);
    }

    return solucao;
}

// Funcao principal do ACO
sSolucao executarACO(const sGrafo& Grafo, const vector<vector<int>>& vviDistancias, int iNumFormigas, int iNumIteracoes, double dInfluenciaFeromonio, double dInfluenciaHeuristica) {
    int iNumeroVertices = Grafo.iNumVertices + 1;
    map<pair<int, int>, double> feromonio;
    
	for (int i = 0; i < iNumeroVertices; ++i)
		for (int j = 0; j < iNumeroVertices; ++j)
			feromonio[{i, j}] = 1.0;

    sSolucao melhorSolucao;
    melhorSolucao.iCustoTotal = INF;

    // Critério de parada inteligente
    const int MAX_SEM_MELHORA = 50;
    int semMelhora = 0;
    int ultimaMelhor = -1;

    for (int iteracao = 0; iteracao < iNumIteracoes; ++iteracao) {
        vector<sSolucao> vsPopulacaoSolucoes;

        for (int formiga = 0; formiga < iNumFormigas; ++formiga) {
            sSolucao solucao = construirSolucao(Grafo, vviDistancias, feromonio);
            if (solucao.iCustoTotal < melhorSolucao.iCustoTotal) {
                melhorSolucao = solucao;
                semMelhora = 0;
                ultimaMelhor = iteracao;
            }
            vsPopulacaoSolucoes.push_back(solucao);
        }

        // Evaporacao
        for (auto& par : feromonio)
			par.second *= (1.0 - TAXA_EVAPORACAO_FEROMONIO);

        // Atualiza feromônio com base nas solucoes
        for (const auto& solucao : vsPopulacaoSolucoes) {
            for (const auto& rota : solucao.rotas) {
                int atual = Grafo.deposito;
                for (const auto& servico : rota.vsServicos) {
                    feromonio[{atual, servico.iVertice1}] += 1.0 / solucao.iCustoTotal;
                    atual = servico.iVertice2;
                }
                feromonio[{atual, Grafo.deposito}] += 1.0 / solucao.iCustoTotal;
            }
        }
        
        // Reforco da melhor solucao global (elitismo)
        for (const auto& rota : melhorSolucao.rotas) {
            int atual = Grafo.deposito;
            for (const auto& servico : rota.vsServicos) {
                feromonio[{atual, servico.iVertice1}] += 1.0 / melhorSolucao.iCustoTotal;
                atual = servico.iVertice2;
            }
            feromonio[std::make_pair(atual, Grafo.deposito)] += 1.0 / melhorSolucao.iCustoTotal;
        }

        semMelhora++;
        if (semMelhora >= MAX_SEM_MELHORA) {
            cout << "Parando antecipadamente por falta de melhora apos " << MAX_SEM_MELHORA << " iteracoes.\n";
            break;
        }
    }

    return melhorSolucao;
}

// Imprime uma única rota
void imprimirRota(int index, const sRota& rota) {
    cout << "Rota " << index+1
              << " | Custo: " << rota.custoTotal
              << " | Demanda total: " << rota.demandaTotal
              << "\n  Servicos:\n";
    for (const auto& servico : rota.vsServicos) {
        cout << "    - Servico " << servico.id
			 << " (" << servico.iVertice1 << "->" << servico.iVertice2 << ")"
			 << " | Demanda: " << servico.demanda
			 << " | Custo atendimento: " << servico.custo
			 << endl;
    }
}

// Imprime a solucao completa
void imprimirSolucao(const sSolucao& sSolucao) {
    cout << "=== Solucao ACO ===" << endl;
    cout << "Custo total da solucao: " << sSolucao.iCustoTotal << endl << endl;
    
    for (int i = 0; i < (int)sSolucao.rotas.size(); ++i) {
        imprimirRota(i, sSolucao.rotas[i]);
        cout << endl;
    }
}

void salvarSolucaoDat(
						 const sSolucao& sSolucao,
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

    // 1) cabecalho
    ofs << sSolucao.iCustoTotal        << "\n"  // custo total da solucao
        << sSolucao.rotas.size()      << "\n"  // total de rotas
        << lClockExecucaoReferencia   << "\n"  // clocks na execucao do alg ref
        << lClockParaAcharSolucaoRef  << "\n"; // clocks até achar solucao

    // 2) cada rota
    for (int i = 0; i < (int)sSolucao.rotas.size(); ++i) {
        const sRota& rota = sSolucao.rotas[i];
        
        int idRota       = int(i) + 1;
        int visitas      = int(rota.vsServicos.size()) + 2; 
        // +2 porque inclui visita inicial e final ao deposito

        // linha básica: deposito, dia, idRota, demanda total, custo total, visitas
        ofs << " " 
            << 0  		         << " "
            << iDia       	     << " "
            << idRota    		 << " "
            << rota.demandaTotal << " "
            << rota.custoTotal   << "  "
            << visitas;

        // tripla inicial de deposito
        ofs << " (D " << 0 << "," << iDeposito << "," << iDeposito << ")";

        // para cada servico
        for (const auto& servico : rota.vsServicos) {
            ofs << " (S " 
                << servico.id << ","    // identificador do servico
                << servico.iVertice1  << ","    // extremidade de partida
                << servico.iVertice2  << ")";   // extremidade de chegada
        }

        // tripla final de deposito
        ofs << " (D " << 0 << "," << iDeposito << "," << iDeposito << ")\n";
    }

    ofs.close();
}
