# Trabalho de IA: Reversão do Jogo da Vida do Conway

Equipe:
-   Tiago Mendes Bottamedi (GRR2022)
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

    [Explicar melhor esse exemplo ae]

-   [Explicar das restrições sobre a borda]
-   [Explicar da tática de criar uma borda virtual]

-   Além de tentar encontrar uma valoração que satisfaça as restrições, o
    OpenWBO paraleliza a busca a fim de encontrar uma valoração que minimize o
    número de células vivas.
-   Depois de 300 segundos, todas as threads morrem e ele imprime a melhor
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

-   Por fim, a matriz em T0 é imprimida.


