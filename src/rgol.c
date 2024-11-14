#include <cvc5/c/cvc5.h>
#include <stdio.h>
#include <stdlib.h>

#include "rgol.h"
#include "utils.h"

// read_board lê o tabuleiro do jogo da vida.
// retorno:
// - 0: sucesso
// - 1: erro
__always_inline
int read_board(struct rgol* r) {
	for (int i = 0; i < r->lin * r->col; i++){
		if (scanf("%d", &r->board_t1[i]) != 1) {
			return 1;
		}
	}
	return 0;
}

__always_inline
int rgol_init(struct rgol* r) {
	if (scanf("%d", &r->lin) != 1 || scanf("%d", &r->col) != 1) return 1;

	if (read_board(r) != 0) return 1;

	r->tm = cvc5_term_manager_new();
	r->slv = cvc5_new(r->tm);

	cvc5_set_option(r->slv, "produce-models", "true");
	cvc5_set_option(r->slv, "produce-unsat-cores", "true");
	cvc5_set_logic(r->slv, "ALL");

	r->zero = cvc5_mk_integer_int64(r->tm, 0);
	r->one = cvc5_mk_integer_int64(r->tm, 1);
	r->two = cvc5_mk_integer_int64(r->tm, 2);
	r->three = cvc5_mk_integer_int64(r->tm, 3);

	Cvc5Sort int_sort = cvc5_get_integer_sort(r->tm);
	char name[20];
	for (int i = 0; i < r->lin * r->col; i++) {
		sprintf(name, "x%d", i);
		r->board_t0[i] = cvc5_mk_const(r->tm, int_sort, name);

		// T0[i] == 0
		Cvc5Term zero_constraint = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, (Cvc5Term[]){r->board_t0[i], r->zero});
		// T0[i] == 1
		Cvc5Term one_constraint = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, (Cvc5Term[]){r->board_t0[i], r->one});
		// (T0[i] == 0) or (T0[i] == 1)
		Cvc5Term constraint = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, (Cvc5Term[]){zero_constraint, one_constraint});

		cvc5_assert_formula(r->slv, constraint);
	}

	return 0;
}

__always_inline
void rgol_destroy(struct rgol* r) {
	cvc5_delete(r->slv);
	cvc5_term_manager_delete(r->tm);
}

// handle_board_edges é o código para as 4 quinas.
__always_inline
void handle_board_edges(struct rgol* r) {
	for (int i = 0; i < r->lin; i += r->lin-1){
		for (int j = 0; j < r->col; j += r->col-1){
			Cvc5Term sum[3];
			if (i == 0){
				if (j == 0){
					sum[0] = r->board_t0[1];
					sum[1] = r->board_t0[r->col];
					sum[2] = r->board_t0[r->col+1];
				}
				else{
					sum[0] = r->board_t0[r->col-2];
					sum[1] = r->board_t0[2*r->col-1];
					sum[2] = r->board_t0[2*r->col-2];
				}
			}
			else{
				if (j == 0){
					sum[0] = r->board_t0[i*r->col+1];
					sum[1] = r->board_t0[(i-1)*r->col];
					sum[2] = r->board_t0[(i-1)*r->col+1];
				}
				else{
					sum[0] = r->board_t0[(i+1)*r->col-2];
					sum[1] = r->board_t0[i*r->col-1];
					sum[2] = r->board_t0[i*r->col-2];
				}
			}
			Cvc5Term args[2];
			args[0] = cvc5_mk_term(r->tm, CVC5_KIND_ADD, 3, sum);
			Cvc5Term formula[2];
			args[1] = r->two;
			Cvc5Term aux[2] = {r->board_t0[i*r->col+j], r->one};
			aux[0] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, aux);
			aux[1] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2 , args);
			Cvc5Term and[1] = {cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, aux)};
			formula[0] = cvc5_mk_term(r->tm, CVC5_KIND_NOT, 1, and);
			args[1] = r->three;
			formula[1] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, args);
			Cvc5Term c = cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, formula);
			cvc5_assert_formula(r->slv, c);
		}
	}
	for (int j = 1; j < r->col-1; j++){
		for (int i = 0; i < r->lin; i+=(r->lin-1)){
			int neighbours;
			Cvc5Term sum[5];
			if (i == 0){
				neighbours = r->board_t1[j-1] + r->board_t1[j+1] + r->board_t1[r->col+j-1] + r->board_t1[r->col+j] + r->board_t1[r->col+j+1];
				sum[0] = r->board_t0[j-1];
				sum[1] = r->board_t0[j+1];
				sum[2] = r->board_t0[r->col+j-1];
				sum[3] = r->board_t0[r->col+j];
				sum[4] = r->board_t0[r->col+j+1];
			} else {
				neighbours = r->board_t1[i*r->col+j-1] + r->board_t1[i*r->col+j+1] + r->board_t1[(i-1)*r->col+j-1] + r->board_t1[(i-1)*r->col+j] + r->board_t1[(i-1)*r->col+j+1];
				sum[0] = r->board_t0[i*r->col+j-1];
				sum[1] = r->board_t0[i*r->col+j+1];
				sum[2] = r->board_t0[(i-1)*r->col+j-1];
				sum[3] = r->board_t0[(i-1)*r->col+j];
				sum[4] = r->board_t0[(i-1)*r->col+j+1];
			}
			Cvc5Term args[2];
			args[0] = cvc5_mk_term(r->tm, CVC5_KIND_ADD, 5, sum);
			Cvc5Term formula[2];
			args[1] = r->two;
			Cvc5Term aux[2] = {r->board_t0[i*r->col+j], r->one};
			aux[0] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, aux);
			aux[1] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2 , args);
			Cvc5Term and[1] = {cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, aux)};
			formula[0] = cvc5_mk_term(r->tm, CVC5_KIND_NOT, 1, and);
			args[1] = r->three;
			formula[1] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, args);
			Cvc5Term c = cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, formula);
			cvc5_assert_formula(r->slv, c);
		}
	}
	for (int i = 1; i < r->lin - 1; i++){
		for(int j = 0; j < r->col; j+=(r->col-1)){
			int neighbours;
			Cvc5Term sum[5];
			if (j == 0){
				neighbours = r->board_t1[(i-1)*r->col] + r->board_t1[(i-1)*r->col+1] + r->board_t1[i*r->col+1] + r->board_t1[(i+1)*r->col] + r->board_t1[(i+1)*r->col+1];
				sum[0] = r->board_t0[(i-1)*r->col];
				sum[1] = r->board_t0[(i-1)*r->col+1];
				sum[2] = r->board_t0[i*r->col+1];
				sum[3] = r->board_t0[(i+1)*r->col];
				sum[4] = r->board_t0[(i+1)*r->col+1];
			} else{
				neighbours = r->board_t1[(i-1)*r->col+j] + r->board_t1[(i-1)*r->col+j-1] + r->board_t1[i*r->col+j-1] + r->board_t1[(i+1)*r->col+j-1] + r->board_t1[(i+1)*r->col+j];
				sum[0] = r->board_t0[(i-1)*r->col+j];
				sum[1] = r->board_t0[(i-1)*r->col+j-1];
				sum[2] = r->board_t0[i*r->col+j-1];
				sum[3] = r->board_t0[(i+1)*r->col+j-1];
				sum[4] = r->board_t0[(i+1)*r->col+j];
			}
			Cvc5Term args[2];
			args[0] = cvc5_mk_term(r->tm, CVC5_KIND_ADD, 5, sum);
			Cvc5Term formula[2];
			args[1] = r->two;
			Cvc5Term aux[2] = {r->board_t0[i*r->col+j], r->one};
			aux[0] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, aux);
			aux[1] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2 , args);
			Cvc5Term and[1] = {cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, aux)};
			formula[0] = cvc5_mk_term(r->tm, CVC5_KIND_NOT, 1, and);
			args[1] = r->three;
			formula[1] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, args);
			Cvc5Term c = cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, formula);
			cvc5_assert_formula(r->slv, c);
		}
	}
}

__always_inline
void rgol_make_formulas(struct rgol* r) {
	for (int i = 1; i < r->lin - 1; i++) {
		for (int j = 1; j < r->col - 1; j++) {
			Cvc5Term sum[8] = {
				r->board_t0[(i-1)*r->col+j], r->board_t0[(i-1)*r->col+j-1], r->board_t0[(i-1)*r->col+j+1],
			 	r->board_t0[i*r->col+j-1],                                  r->board_t0[i*r->col+j+1],
			 	r->board_t0[(i+1)*r->col+j], r->board_t0[(i+1)*r->col+j-1], r->board_t0[(i+1)*r->col+j+1]
			};

			Cvc5Term args[2];
			args[0] = cvc5_mk_term(r->tm, CVC5_KIND_ADD, 8, sum);
			args[1] = r->two;

			Cvc5Term aux[2] = {r->board_t0[i*r->col+j], r->one};
			// t0[i][j] == 1
			aux[0] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, aux);
			// sum == 2
			aux[1] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2 , args);

			Cvc5Term formula[2], c;
			if (r->board_t1[i*r->col + j] == 0) {
				// (t0[i][j] == 1) and (sum == 2)
				Cvc5Term and[1] = {cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, aux)};
				// !((t0[i][j] == 1) and (sum == 2))
				formula[0] = cvc5_mk_term(r->tm, CVC5_KIND_NOT, 1, and);
				args[1] = r->three;
				// sum != 3
				formula[1] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, args);
				// !((t0[i][j] == 1) and (sum == 2)) and (sum != 3)
				c = cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, formula);
			} else {
				// (t0[i][j] == 1) and (sum == 2)
				formula[0] = cvc5_mk_term(r->tm, CVC5_KIND_AND, 2, aux);
				args[1] = r->three;
				// sum == 3
				formula[1] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, args);
				// (t0[i][j] == 1) and (sum == 2) or (sum == 3)
				c = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, formula);
			}
			cvc5_assert_formula(r->slv, c);
		}
	}

	handle_board_edges(r);
	print_debug("acabei de montar");
}
