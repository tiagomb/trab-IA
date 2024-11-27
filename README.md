# Trabalho de IA: Reversão do Jogo da Vida do Conway

Equipe:
-   Tiago Mendes Bottamedi (GRR20220068)
-   Yago Yudi Vilela Furuta (GRR20221231)

## Instruções de Uso

```sh
cd src

make

./main
```

Dependências:
- compilador de c++
- make
- zlib
- gmpxx

## Estrutura do projeto

-   `papers` contém os artigos que usamos como referência.
-   `src` contém o código fonte.
    - Boa parte do código fonte é composto pelo código fonte do OpenWBO, que é
      o SAT solver que escolhemos usar.
    - Foi modificado somente a `Main.cc` do OpenWBO.

## Descrição da estratégia implementada

-   Este programa realiza a reversão do Jogo da Vida (Conway's Game of Life)
    com o objetivo de encontrar um estado anterior que minimize o número de
    células vivas. 
-   A solução utiliza uma abordagem baseada em satisfatibilidade booleana.
-   As restrições booleanas implementadas foram as descritas no artigo
    "Time-Reversal in Conway's Life as SAT" do Stuart Bain. 
-   O artigo está disponível no moodle da disciplina e em
    `./papers/life-time-reversal-sat.pdf`.
-   Essas restrições foram escritas usando a API do SAT solver OpenWBO
    (github.com/sat-group/open-wbo). Dessa forma, o OpenWBO tenta encontrar uma
    valoração que satisfaça as restrições.

    Por exemplo, a fórmula de Loneliness do artigo foi traduzida em:

    ```c
    addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false)});
    addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j+1,false)});
    addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
    addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j+1,false)});
    addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
    addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
    addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
    addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
    ```

    Nesse caso, todas as 8 permutações de 7 vizinhos possíveis foram consideradas, com todas as variáveis não negadas, o que é representado pelo parâmetro "false" (a variável normal ser indicada por "false" e a negada por "true" é um pouco contraintuitivo, porém é a convenção utilizada)

-   Para contemplar as bordas, as regras para células mortas foram aplicadas considerando o número de vizinhos. Cada 1 dos 4 cantos possui 3 vizinhos, enquanto as  demais células possuem 5 vizinhos.

-   O SAT solver utilizado "por debaixo dos panos" é o Glucose, a opção padrão. Testes realizados com os outros solvers disponíveis não demonstraram diferenças significativas.

-   O algoritmo escolhido dentre os disponíveis no OpenWBO foi o PartMSU3, uma versão do MSU3 que trabalha com partições. Ele foi escolhido por apresentar a melhor velocidade de resposta, obtendo o mesmo resultado ou resultados melhores que os demais algoritmos para o problema de reverter o jogo da vida. 

-   Além de tentar encontrar uma valoração que satisfaça as restrições, o
    algoritmo paraleliza a busca a fim de encontrar uma valoração que minimize o
    número de células vivas.
-   Depois de 300 segundos, ele imprime a melhor
    solução encontrada.

## Saída do programa

O OpenWBO segue o padrão de saída dos MaxSAT solvers:
-   "c" para comentários.
-   "s" para status da solução.
    - OPTIMUM FOUND
    - UNSATISFIABLE
    - SATISFIABLE
-   "o" para o custo da solução.
    
    Aqui o custo da solução é a quantidade de células vivas. Assim, quanto
    menor o custo, menor a quantidade de células vivas.

-   Por fim, a matriz em T0 é impressa.


