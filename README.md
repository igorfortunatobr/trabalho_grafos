# 🔍 Trabalho Prático Final - GCC262: Grafos e suas Aplicações

**Alunos:**  
- Igor Abreu Fortunato  
- Luiza Oliveira De Souza Garcia  

## 📌 Descrição do Projeto

Este projeto corresponde à **1ª etapa** do trabalho prático da disciplina GCC262 - Grafos e suas Aplicações. Seu objetivo é realizar o **pré-processamento e análise de instâncias de grafos**, extraindo estatísticas relevantes que servirão de base para etapas posteriores.

---

## ⚙️ Funcionalidades Implementadas

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

---

## 🧪 Como Executar

1. Clone este repositório:
   ```bash
   git clone https://github.com/igorfortunatobr/trabalho_grafos.git
    ```

2. Compile o programa

3. Execute o programa a partir de main.cpp

4. Quando solicitado, digite o nome de um dos arquivos da pasta instancias/, por exemplo:
    ```bash
    BHW1.dat
    ```
5. O programa exibirá as estatísticas no terminal e salvará os resultados em um arquivo .json dentro da pasta estatisticas/.

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
├── estatisticas/           # Arquivos .json gerados com as estatísticas
├── instancias/             # Instâncias de entrada (formato .dat)
├── estatisticas.cpp        # Cálculos e funções estatísticas
├── leitura.cpp             # Função para leitura e parsing dos arquivos
├── grafo.hpp               # Definições das estruturas de dados
├── main.cpp                # Ponto de entrada do programa
├── visualizacao.ipynb      # Notebook para visualização dos resultados
└── README.md               # Este arquivo
```

## 🎓 Observações Finais

Este trabalho foi desenvolvido com fins acadêmicos para a disciplina GCC262 - Grafos e suas Aplicações.
A modularidade do código permite que as funcionalidades sejam facilmente expandidas para as próximas etapas do projeto.
