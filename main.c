#include <cvc5/c/cvc5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    int lin, col;
    scanf ("%d", &lin);
    scanf ("%d", &col);
    int *m = malloc(sizeof(int) * lin * col);
    for (int i = 0; i < lin * col; i++){
        scanf ("%d", &m[i]);
    }
    Cvc5TermManager* tm = cvc5_term_manager_new();
    Cvc5* slv = cvc5_new(tm);
    cvc5_set_option(slv, "produce-models", "true");
    cvc5_set_option(slv, "produce-unsat-cores", "true");
    cvc5_set_logic(slv, "ALL");
    Cvc5Sort int_sort = cvc5_get_integer_sort(tm);
    Cvc5Term x[lin * col];
    Cvc5Term zero = cvc5_mk_integer_int64(tm, 0);
    Cvc5Term one = cvc5_mk_integer_int64(tm, 1);
    Cvc5Term two = cvc5_mk_integer_int64(tm, 2);
    Cvc5Term three = cvc5_mk_integer_int64(tm, 3);
    char name[20];
    for (int i = 0; i < lin * col; i++){
        sprintf(name, "x%d", i);
        x[i] = cvc5_mk_const(tm, int_sort, name);
        Cvc5Term zero_constraint = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, (Cvc5Term[]){x[i], zero});
        Cvc5Term one_constraint = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, (Cvc5Term[]){x[i], one});
        Cvc5Term constraint = cvc5_mk_term(tm, CVC5_KIND_OR, 2, (Cvc5Term[]){zero_constraint, one_constraint});
        cvc5_assert_formula(slv, constraint);
    }
    for (int i = 1; i < lin - 1; i++){
        for (int j = 1; j < col - 1; j++){
            Cvc5Term sum[8] = {x[(i-1)*col+j], x[(i-1)*col+j-1], x[(i-1)*col+j+1], x[i*col+j-1], x[i*col+j+1], x[(i+1)*col+j], x[(i+1)*col+j-1], x[(i+1)*col+j+1]};
            Cvc5Term args[2];
            args[0] = cvc5_mk_term(tm, CVC5_KIND_ADD, 8, sum);
            if (m[i*col+j] == 0){
                Cvc5Term formula[3];
                args[1] = three;
                Cvc5Term aux[2] = {x[i*col+j], zero};
                aux[0] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, aux);
                aux[1] = cvc5_mk_term(tm, CVC5_KIND_DISTINCT, 2, args);
                formula[0] = cvc5_mk_term(tm, CVC5_KIND_AND, 2, aux);
                formula[1] = cvc5_mk_term(tm, CVC5_KIND_GT, 2, args);
                args[1] = two;
                formula[2] = cvc5_mk_term(tm, CVC5_KIND_LT, 2, args);
                Cvc5Term c = cvc5_mk_term(tm, CVC5_KIND_OR, 3 , formula);
                cvc5_assert_formula(slv, c);
            } else {
                Cvc5Term formula[2];
                args[1] = two;
                Cvc5Term aux[2] = {x[i*col+j], one};
                aux[0] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, aux);
                aux[1] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2 , args);
                formula[0] = cvc5_mk_term(tm, CVC5_KIND_AND, 2, aux);
                args[1] = three;
                formula[1] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, args);
                Cvc5Term c = cvc5_mk_term(tm, CVC5_KIND_OR, 2, formula);
                cvc5_assert_formula(slv, c);
            }
        }
    }
    Cvc5Result r = cvc5_check_sat(slv);
    printf("result: %s\n", cvc5_result_to_string(r));
    for (int i = 0; i < lin; i++){
        for (int j = 0; j < col; j++){
            Cvc5Term value = cvc5_get_value(slv, x[i*col+j]);
            const char *val = cvc5_term_to_string(value);
            printf ("%s ", val);
        }
        printf ("\n");
    }
    cvc5_delete(slv);
    cvc5_term_manager_delete(tm);
    return 0;
}