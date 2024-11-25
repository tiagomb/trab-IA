#include "EvalMaxSAT/lib/EvalMaxSAT/src/EvalMaxSAT.h"
#include <stdio.h>

#define MAX_SIZE 625

int main(int argc, char *argv[]) {
    EvalMaxSAT solver;
    int lin, col;
    int board_t1[MAX_SIZE];
    int board_t0[MAX_SIZE];
    scanf ("%d", &lin);
    scanf ("%d", &col);
    for (int i = 0; i < lin*col;i++){
        scanf("%d", &board_t1[i]);
        board_t0[i] = solver.newVar();
    }
    for (int i = 1; i < lin-1; i++){
        for (int j = 1; j < col-1; j++){
            if (board_t1[i*col+j] == 1){
                //{board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]}
                //Loneliness = C(8,7) = 8!/7! = 8 possibilidades
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                //Stagnation = C(8,2) = 8!/6!2! = 28 possiblidades
                solver.addClause({board_t0[i*col+j], -board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                //Overcrowding = C(8,4)= 8!/4!4! = 70 possibilidades
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[i*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1]});
                solver.addClause({-board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j-1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
            } else {
                //Preservation = C(8,2) = 8!/6!2! = 28 possibilidades
                solver.addClause({-board_t0[i*col+j], -board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], -board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[i*col+j], board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                //Life = C(8,3) = 8!/5!3! = 56 possibilidades
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({-board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], -board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], -board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], -board_t0[i*col+j-1], board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], -board_t0[(i+1)*col+j], board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], -board_t0[i*col+j+1], board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
                solver.addClause({board_t0[(i-1)*col+j], board_t0[(i-1)*col+j-1], board_t0[(i-1)*col+j+1], board_t0[i*col+j-1], board_t0[i*col+j+1], -board_t0[(i+1)*col+j], -board_t0[(i+1)*col+j-1], -board_t0[(i+1)*col+j+1]});
            }
        }
    }
    for (int i = 0; i < lin*col; i++){
        solver.addClause({-board_t0[i]}, 1);
    }
    ////// PRINT SOLUTION //////////////////
    if(!solver.solve()) {
        std::cout << "s UNSATISFIABLE" << std::endl;
        return 0;
    }
    std::cout << "s OPTIMUM FOUND" << std::endl;
    std::cout << "o " << solver.getCost() << std::endl;
    for (int i = 0; i < lin; i++){
        for (int j = 0; j < col; j++){
            std::cout << solver.getValue(board_t0[i*col+j]) << " ";
        }
        std::cout << "\n";
    }
    ///////////////////////////////////////
}