# 🔍 Trabalho Prático Final - GCC262: Grafos e suas Aplicações

**Alunos:**  
- Igor Abreu Fortunato  
- Luiza Oliveira De Souza Garcia  

## 📌 Descrição do Projeto

Este projeto corresponde às **etapas 1 e 2** do trabalho prático da disciplina GCC262 - Grafos e suas Aplicações.  
Seu objetivo é realizar:

- **Etapa 1:** Pré-processamento e análise de instâncias de grafos, extraindo estatísticas relevantes;
- **Etapa 2:** Geração de uma solução inicial para o problema por meio de um algoritmo construtivo baseado na **Colônia de Formigas (ACO - Ant Colony Optimization)**.
---

## ⚙️ Funcionalidades Implementadas

### 🧮 Etapa 1 – Estatísticas do Grafo

A partir de arquivos de entrada, o programa realiza:

- Leitura e modelagem dos dados em estruturas de grafos;
- Geração da matriz de distâncias e predecessores utilizando o algoritmo de Floyd-Warshall;
- Cálculo das seguintes estatísticas:

1. Quantidade de vértices  
2. Quantidade de arestas  
3. Quantidade de arcos  
4. Quantidade de vértices requeridos  
5. Quantidade de arestas requeridas  
6. Quantidade de arcos requeridos  
7. Densidade do grafo (order strength)  
8. Componentes conectados  
9. Grau mínimo e máximo dos vértices  
10. Intermediação (betweenness)  
11. Caminho médio  
12. Diâmetro  

Os resultados são exibidos no terminal e salvos em arquivos `.json` na pasta `estatisticas/`.

### 🐜 Etapa 2 – Solução Inicial com Colônia de Formigas (ACO)

Foi implementado um algoritmo construtivo baseado em Colônia de Formigas que:

- Considera a capacidade dos veículos;
- Garante que cada serviço (nó, aresta ou arco requerido) seja atendido por uma rota;
- Calcula o custo total das rotas e respeita a contagem única de demandas e custos;
- Utiliza feromônio e heurística para guiar a construção de rotas otimizadas;
- Salva a solução gerada em arquivos `.dat` seguindo o formato especificado.

---

## 🧪 Como Executar

1. Clone este repositório:
   ```bash
   git clone https://github.com/igorfortunatobr/trabalho_grafos.git
    ```

2. Compile o programa

3. Execute o programa a partir de main.cpp

4. O sistema processará automaticamente os arquivos listados em arquivosInstancia.hpp

5. O programa irá:

- Exibir as estatísticas da instância no terminal;
- Salvar as estatísticas no diretório estatisticas/;
- Executar o algoritmo de colônia de formigas;
- Salvar a solução inicial no diretório solucoes/.

## 📊 Visualização dos Resultados

Para visualizar graficamente as informações geradas:

1. Acesse Google Colab;
2. Faça upload do notebook **visualizacao.ipynb** e do **arquivo .json** gerado em **estatisticas/**;
3. No notebook, altere a variável **nome_arquivo** para o nome correto do arquivo .json;
4. Execute todas as células para visualizar o grafo e as estatísticas em formato gráfico.

## 🛠️ Requisitos

- Compilador C++ (como g++)
- Acesso ao Google Colab para abrir o notebook visualizacao.ipynb e analisar os arquivos .json gerados

## 📁 Estrutura de Diretórios

```bash
├── estatisticas/           # Arquivos .json com as estatísticas do grafo
├── solucoes/               # Arquivos .dat com as soluções geradas pelo ACO
├── instancias/             # Instâncias de entrada (formato .dat)
├── estatisticas.cpp        # Cálculos e funções estatísticas
├── leitura.cpp             # Leitura e parsing dos arquivos de instância
├── colonia_formigas.cpp    # Implementação do algoritmo ACO
├── grafo.hpp               # Estrutura de dados para o grafo
├── main.cpp                # Ponto de entrada principal
├── arquivosInstancia.hpp   # Lista de arquivos a serem processados
├── visualizacao.ipynb      # Notebook de visualização gráfica
└── README.md               # Este arquivo
```

## 🎓 Observações Finais

Este trabalho foi desenvolvido com fins acadêmicos para a disciplina GCC262 - Grafos e suas Aplicações.
A modularidade do código permite que as funcionalidades sejam facilmente expandidas para as próximas etapas do projeto.
