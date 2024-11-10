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
            Cvc5Term formula[2];
            args[1] = two;
            Cvc5Term aux[2] = {x[i*col+j], one};
            aux[0] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, aux);
            aux[1] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2 , args);
            if (m[i*col+j] == 0){
                Cvc5Term and[1] = {cvc5_mk_term(tm, CVC5_KIND_AND, 2, aux)};
                formula[0] = cvc5_mk_term(tm, CVC5_KIND_NOT, 1, and);
                args[1] = three;
                formula[1] = cvc5_mk_term(tm, CVC5_KIND_DISTINCT, 2, args);
                Cvc5Term c = cvc5_mk_term(tm, CVC5_KIND_AND, 2, formula);
                cvc5_assert_formula(slv, c);
            } else {
                formula[0] = cvc5_mk_term(tm, CVC5_KIND_AND, 2, aux);
                args[1] = three;
                formula[1] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, args);
                Cvc5Term c = cvc5_mk_term(tm, CVC5_KIND_OR, 2, formula);
                cvc5_assert_formula(slv, c);
            }
        }
    }
    //Código para as 4 quinas
    for (int i = 0; i < lin; i += lin-1){
        for (int j = 0; j < col; j += col-1){
            Cvc5Term sum[3];
            if (i == 0){
                if (j == 0){
                    sum[0] = x[1];
                    sum[1] = x[col];
                    sum[2] = x[col+1];
                }
                else{
                    sum[0] = x[col-2];
                    sum[1] = x[2*col-1];
                    sum[2] = x[2*col-2];
                }
            }
            else{
                if (j == 0){
                    sum[0] = x[i*col+1];
                    sum[1] = x[(i-1)*col];
                    sum[2] = x[(i-1)*col+1];
                }
                else{
                    sum[0] = x[(i+1)*col-2];
                    sum[1] = x[i*col-1];
                    sum[2] = x[i*col-2];
                }
            }
            Cvc5Term args[2];
            args[0] = cvc5_mk_term(tm, CVC5_KIND_ADD, 3, sum);
            Cvc5Term formula[2];
            args[1] = two;
            Cvc5Term aux[2] = {x[i*col+j], one};
            aux[0] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, aux);
            aux[1] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2 , args);
            Cvc5Term and[1] = {cvc5_mk_term(tm, CVC5_KIND_AND, 2, aux)};
            formula[0] = cvc5_mk_term(tm, CVC5_KIND_NOT, 1, and);
            args[1] = three;
            formula[1] = cvc5_mk_term(tm, CVC5_KIND_DISTINCT, 2, args);
            Cvc5Term c = cvc5_mk_term(tm, CVC5_KIND_AND, 2, formula);
            cvc5_assert_formula(slv, c);
        }
    }
    for (int j = 1; j < col-1; j++){
        for (int i = 0; i < lin; i+=(lin-1)){
            int neighbours;
            Cvc5Term sum[5];
            if (i == 0){
                neighbours = m[j-1]+m[j+1]+m[col+j-1]+m[col+j]+m[col+j+1];
                sum[0] = x[j-1];
                sum[1] = x[j+1];
                sum[2] = x[col+j-1];
                sum[3] = x[col+j];
                sum[4] = x[col+j+1];
            } else {
                neighbours = m[i*col+j-1]+m[i*col+j+1]+m[(i-1)*col+j-1]+m[(i-1)*col+j]+m[(i-1)*col+j+1];
                sum[0] = x[i*col+j-1];
                sum[1] = x[i*col+j+1];
                sum[2] = x[(i-1)*col+j-1];
                sum[3] = x[(i-1)*col+j];
                sum[4] = x[(i-1)*col+j+1];
            }
            Cvc5Term args[2];
            args[0] = cvc5_mk_term(tm, CVC5_KIND_ADD, 5, sum);
            Cvc5Term formula[2];
            args[1] = two;
            Cvc5Term aux[2] = {x[i*col+j], one};
            aux[0] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, aux);
            aux[1] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2 , args);
            Cvc5Term and[1] = {cvc5_mk_term(tm, CVC5_KIND_AND, 2, aux)};
            formula[0] = cvc5_mk_term(tm, CVC5_KIND_NOT, 1, and);
            args[1] = three;
            formula[1] = cvc5_mk_term(tm, CVC5_KIND_DISTINCT, 2, args);
            Cvc5Term c = cvc5_mk_term(tm, CVC5_KIND_AND, 2, formula);
            cvc5_assert_formula(slv, c);
        }
    }
    for (int i = 1; i < lin - 1; i++){
        for(int j = 0; j < col; j+=(col-1)){
            int neighbours;
            Cvc5Term sum[5];
            if (j == 0){
                neighbours = m[(i-1)*col]+m[(i-1)*col+1]+m[i*col+1]+m[(i+1)*col]+m[(i+1)*col+1];
                sum[0] = x[(i-1)*col];
                sum[1] = x[(i-1)*col+1];
                sum[2] = x[i*col+1];
                sum[3] = x[(i+1)*col];
                sum[4] = x[(i+1)*col+1];
            } else{
                neighbours = m[(i-1)*col+j]+m[(i-1)*col+j-1]+m[i*col+j-1]+m[(i+1)*col+j-1]+m[(i+1)*col+j];
                sum[0] = x[(i-1)*col+j];
                sum[1] = x[(i-1)*col+j-1];
                sum[2] = x[i*col+j-1];
                sum[3] = x[(i+1)*col+j-1];
                sum[4] = x[(i+1)*col+j];
            }
            Cvc5Term args[2];
            args[0] = cvc5_mk_term(tm, CVC5_KIND_ADD, 5, sum);
            Cvc5Term formula[2];
            args[1] = two;
            Cvc5Term aux[2] = {x[i*col+j], one};
            aux[0] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2, aux);
            aux[1] = cvc5_mk_term(tm, CVC5_KIND_EQUAL, 2 , args);
            Cvc5Term and[1] = {cvc5_mk_term(tm, CVC5_KIND_AND, 2, aux)};
            formula[0] = cvc5_mk_term(tm, CVC5_KIND_NOT, 1, and);
            args[1] = three;
            formula[1] = cvc5_mk_term(tm, CVC5_KIND_DISTINCT, 2, args);
            Cvc5Term c = cvc5_mk_term(tm, CVC5_KIND_AND, 2, formula);
            cvc5_assert_formula(slv, c);
        }
    }
    printf("Acabei de montar\n");
    Cvc5Result r = cvc5_check_sat(slv);
    printf("result: %s\n", cvc5_result_to_string(r));
    if (cvc5_result_is_sat(r)){
        for (int i = 0; i < lin; i++){
            for (int j = 0; j < col; j++){
                Cvc5Term value = cvc5_get_value(slv, x[i*col+j]);
                const char *val = cvc5_term_to_string(value);
                printf ("%s ", val);
            }
            printf ("\n");
        }
    }
    cvc5_delete(slv);
    cvc5_term_manager_delete(tm);
    return 0;
}