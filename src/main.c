#include <cvc5/c/cvc5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rgol.h"

// print_solution imprime uma solução encontrada pelo SAT solver.
__always_inline
static void print_solution(struct rgol* r) {
	for (int i = 0; i < r->lin; i++) {
		for (int j = 0; j < r->col; j++) {
			Cvc5Term value = cvc5_get_value(r->slv, r->board_t0[i*r->col + j]);
			const char* val = cvc5_term_to_string(value);
			printf("%s ", val);
		}
		printf("\n");
	}
}

int main() {
	struct rgol r;

	if (rgol_init(&r) != 0) return 1;

	rgol_make_formulas(&r);

	Cvc5Result result = cvc5_check_sat(r.slv);
	if (cvc5_result_is_sat(result)){
		printf("resultado: sat\n");
		print_solution(&r);
	} else {
		printf("resultado: unsat\n");
	}

	rgol_destroy(&r);

	return 0;
}
