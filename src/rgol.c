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

	char name[20];
	for (int i = 0; i < r->lin * r->col; i++) {
		sprintf(name, "x%d", i);
		r->board_t0[i] = cvc5_mk_const(r->tm, cvc5_get_integer_sort(r->tm), name);

		// t0 == 0 or t0 == 1
		Cvc5Term exp[2], arg[2];
		arg[0] = r->board_t0[i];
		arg[1] = r->zero;
		exp[0] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, arg);
		arg[1] = r->one;
		exp[1] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, arg);
		Cvc5Term constraint = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, exp);

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
			Cvc5Term arg[2], exp[2], constraint1, constraint2;

			// t0 != 1 OR sum != 2
			arg[0] = r->board_t0[i*r->col + j];
			arg[1] = r->one;
			exp[0] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			arg[0] = cvc5_mk_term(r->tm, CVC5_KIND_ADD, 3, sum);
			arg[1] = r->two;
			exp[1] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			constraint1 = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, exp);
			cvc5_assert_formula(r->slv, constraint1);

			// sum != 3
			arg[1] = r->three;
			constraint2 = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			cvc5_assert_formula(r->slv, constraint2);
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
			Cvc5Term arg[2], exp[2], constraint1, constraint2;

			// t0 != 1 OR sum != 2
			arg[0] = r->board_t0[i*r->col + j];
			arg[1] = r->one;
			exp[0] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			arg[0] = cvc5_mk_term(r->tm, CVC5_KIND_ADD, 5, sum);
			arg[1] = r->two;
			exp[1] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			constraint1 = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, exp);
			cvc5_assert_formula(r->slv, constraint1);

			// sum != 3
			arg[1] = r->three;
			constraint2 = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			cvc5_assert_formula(r->slv, constraint2);
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
			Cvc5Term arg[2], exp[2], constraint1, constraint2;

			// t0 != 1 OR sum != 2
			arg[0] = r->board_t0[i*r->col + j];
			arg[1] = r->one;
			exp[0] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			arg[0] = cvc5_mk_term(r->tm, CVC5_KIND_ADD, 5, sum);
			arg[1] = r->two;
			exp[1] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			constraint1 = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, exp);
			cvc5_assert_formula(r->slv, constraint1);

			// sum != 3
			arg[1] = r->three;
			constraint2 = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			cvc5_assert_formula(r->slv, constraint2);
		}
	}
}

__always_inline
void rgol_make_formulas(struct rgol* r) {
	for (int i = 1; i < r->lin - 1; i++) {
		for (int j = 1; j < r->col - 1; j++) {
			Cvc5Term sum_vec[8] = {
				r->board_t0[(i-1)*r->col+j], r->board_t0[(i-1)*r->col+j-1], r->board_t0[(i-1)*r->col+j+1],
			 	r->board_t0[i*r->col+j-1],                                  r->board_t0[i*r->col+j+1],
			 	r->board_t0[(i+1)*r->col+j], r->board_t0[(i+1)*r->col+j-1], r->board_t0[(i+1)*r->col+j+1]
			};
			Cvc5Term sum = cvc5_mk_term(r->tm, CVC5_KIND_ADD, 8, sum_vec);
			Cvc5Term arg[2], exp[2];
			Cvc5Term constraint1, constraint2;
			if (r->board_t1[i*r->col + j] == 0) {
				// t0 != 1 OR sum != 2
				arg[0] = r->board_t0[i*r->col + j];
				arg[1] = r->one;
				exp[0] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
				arg[0] = sum;
				arg[1] = r->two;
				exp[1] = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
				constraint1 = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, exp);

				// sum != 3
				arg[1] = r->three;
				constraint2 = cvc5_mk_term(r->tm, CVC5_KIND_DISTINCT, 2, arg);
			} else {
				// t0 == 1 OR sum == 3
				arg[0] = r->board_t0[i*r->col + j];
				arg[1] = r->one;
				exp[0] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, arg);
				arg[0] = sum;
				arg[1] = r->three;
				exp[1] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, arg);
				constraint1 = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, exp);

				// sum == 2 OR sum == 3
				arg[1] = r->two;
				exp[0] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, arg);
				arg[1] = r->three;
				exp[1] = cvc5_mk_term(r->tm, CVC5_KIND_EQUAL, 2, arg);
				constraint2 = cvc5_mk_term(r->tm, CVC5_KIND_OR, 2, exp);
			}
			cvc5_assert_formula(r->slv, constraint1);
			cvc5_assert_formula(r->slv, constraint2);
		}
	}

	handle_board_edges(r);
	print_debug("acabei de montar");
}
