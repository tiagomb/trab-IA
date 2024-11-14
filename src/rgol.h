#ifndef RGOL
#define RGOL

#include <cvc5/c/cvc5.h>

// O maior tabuleiro é 20x20.
#define MAX_BOARD_SIZE 400

// Reverse Game Of Life.
struct rgol {
	int lin, col;
	int board_t1[MAX_BOARD_SIZE];
	Cvc5Term board_t0[MAX_BOARD_SIZE];

	Cvc5* slv;
	Cvc5TermManager* tm;

	Cvc5Term zero;
	Cvc5Term one;
	Cvc5Term two;
	Cvc5Term three;
};

// rgol_init inicializa os campos da struct rgol.
//
// retorno:
// - 0: sucesso
// - 1: erro
int rgol_init(struct rgol* r);

// rgol_make_formula monta as fórmulas que vão ir para o SAT.
void rgol_make_formulas(struct rgol* r);

// rgol_destroy libera os recursos.
void rgol_destroy(struct rgol* r);

#endif
