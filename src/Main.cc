/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
 * MiniSat,  Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
 *           Copyright (c) 2007-2010, Niklas Sorensson
 * Open-WBO, Copyright (c) 2013-2017, Ruben Martins, Vasco Manquinho, Ines Lynce
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "utils/Options.h"
#include "utils/ParseUtils.h"
#include "utils/System.h"
#include <errno.h>
#include <signal.h>
#include <zlib.h>

#include <fstream>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <string>
#include <vector>

#ifdef SIMP
#include "simp/SimpSolver.h"
#else
#include "core/Solver.h"
#endif

#include "MaxSAT.h"
#include "MaxTypes.h"
#include "ParserMaxSAT.h"
#include "ParserPB.h"

// Algorithms
#include "algorithms/Alg_LinearSU.h"
#include "algorithms/Alg_MSU3.h"
#include "algorithms/Alg_OLL.h"
#include "algorithms/Alg_PartMSU3.h"
#include "algorithms/Alg_WBO.h"

#define VER1_(x) #x
#define VER_(x) VER1_(x)
#define SATVER VER_(SOLVERNAME)
#define VER VER_(VERSION)

using NSPACE::cpuTime;
using NSPACE::OutOfMemoryException;
using NSPACE::IntOption;
using NSPACE::BoolOption;
using NSPACE::StringOption;
using NSPACE::IntRange;
using NSPACE::parseOptions;
using namespace openwbo;

//=================================================================================================

static MaxSAT *mxsolver;
int lin, col;

static void SIGINT_exit(int signum) {
  if (mxsolver->getValue(1) != 0){
    for (int i = 0; i < lin; i++){
      for (int j = 0; j < col; j++){
        if (mxsolver->getValue(i*col+j) < 1)
          printf ("0 ");
        else
          printf ("1 ");
      }
      printf ("\n");
    }
  }
  exit(_UNKNOWN_);
}

//=================================================================================================
#if !defined(_MSC_VER) && !defined(__MINGW32__)
void limitMemory(uint64_t max_mem_mb)
{
// FIXME: OpenBSD does not support RLIMIT_AS. Not sure how well RLIMIT_DATA works instead.
#if defined(__OpenBSD__)
#define RLIMIT_AS RLIMIT_DATA
#endif

    // Set limit on virtual memory:
    if (max_mem_mb != 0){
        rlim_t new_mem_lim = (rlim_t)max_mem_mb * 1024*1024;
        rlimit rl;
        getrlimit(RLIMIT_AS, &rl);
        if (rl.rlim_max == RLIM_INFINITY || new_mem_lim < rl.rlim_max){
            rl.rlim_cur = new_mem_lim;
            if (setrlimit(RLIMIT_AS, &rl) == -1)
                printf("c WARNING! Could not set resource limit: Virtual memory.\n");
        }
    }

#if defined(__OpenBSD__)
#undef RLIMIT_AS
#endif
}
#else
void limitMemory(uint64_t /*max_mem_mb*/)
{
    printf("c WARNING! Memory limit not supported on this architecture.\n");
}
#endif


#if !defined(_MSC_VER) && !defined(__MINGW32__)
void limitTime(uint32_t max_cpu_time)
{
    if (max_cpu_time != 0){
        rlimit rl;
        getrlimit(RLIMIT_CPU, &rl);
        if (rl.rlim_max == RLIM_INFINITY || (rlim_t)max_cpu_time < rl.rlim_max){
            rl.rlim_cur = max_cpu_time;
            if (setrlimit(RLIMIT_CPU, &rl) == -1)
                printf("c WARNING! Could not set resource limit: CPU-time.\n");
        }
    }
}
#else
void limitTime(uint32_t /*max_cpu_time*/)
{
    printf("c WARNING! CPU-time limit not supported on this architecture.\n");
}
#endif

void addLiterals(MaxSATFormula* maxsat_formula, std::initializer_list<Lit> literals) {
    vec<Lit> clause;
    for (const auto& lit : literals) {
        clause.push(lit);  // Add each literal to the vec<Lit>
    }
    maxsat_formula->addHardClause(clause);
}

//=================================================================================================
// Main:

int main(int argc, char **argv) {
#if defined(__linux__)
    fpu_control_t oldcw, newcw;
    _FPU_GETCW(oldcw);
    newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE;
    _FPU_SETCW(newcw);
#endif

    IntOption verbosity("Open-WBO", "verbosity",
                        "Verbosity level (0=minimal, 1=more).\n", 0,
                        IntRange(0, 1));

    IntOption cpu_lim("Open-WBO", "cpu-lim",
                      "Limit on CPU time allowed in seconds.\n", 300,
                      IntRange(0, INT32_MAX));

    IntOption mem_lim("Open-WBO", "mem-lim",
                      "Limit on memory usage in megabytes.\n", 8192,
                      IntRange(0, INT32_MAX));

    IntOption partition_strategy("PartMSU3", "partition-strategy",
                                 "Partition strategy (0=sequential, "
                                 "1=sequential-sorted, 2=binary)"
                                 "(only for unsat-based partition algorithms).",
                                 2, IntRange(0, 2));

    IntOption graph_type("PartMSU3", "graph-type",
                         "Graph type (0=vig, 1=cvig, 2=res) (only for unsat-"
                         "based partition algorithms).",
                         2, IntRange(0, 2));

    IntOption cardinality("Encodings", "cardinality",
                          "Cardinality encoding (0=cardinality networks, "
                          "1=totalizer, 2=modulo totalizer).\n",
                          1, IntRange(0, 2));

    // Try to set resource limits:
    if (cpu_lim != 0) limitTime(cpu_lim);
    if (mem_lim != 0) limitMemory(mem_lim);

    double initial_time = cpuTime();
    MaxSAT *S = NULL;
    S = new PartMSU3(verbosity, partition_strategy, graph_type, cardinality);

    signal(SIGXCPU, SIGINT_exit);
    signal(SIGTERM, SIGINT_exit);

    MaxSATFormula *maxsat_formula = new MaxSATFormula();

    scanf ("%d", &lin);
    scanf ("%d", &col);
    int board_t1[lin*col];
    
    for (int i = 0; i < lin*col; i++){
      scanf ("%d", &board_t1[i]);
      maxsat_formula->newVar();
    }
    maxsat_formula->setFormat(_FORMAT_MAXSAT_);
    for (int i = 1; i < lin-1; i++){
        for (int j = 1; j < col-1; j++){
            if (board_t1[i*col+j] == 1){
                //{mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)}
                //Loneliness = C(8,7) = 8!/7! = 8 possibilidades
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                //Stagnation = C(8,2) = 8!/6!2! = 28 possiblidades
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, false), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                //Overcrowding = C(8,4)= 8!/4!4! = 70 possibilidades
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j-1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
            } else {
                //Preservation = C(8,2) = 8!/6!2! = 28 possibilidades
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit(i*col+j, true), mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                //Life = C(8,3) = 8!/5!3! = 56 possibilidades
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, true), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1, true), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1, true), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, true), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1,false)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1,false), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, true), mkLit((i+1)*col+j, false), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
                addLiterals(maxsat_formula, {mkLit((i-1)*col+j, false), mkLit((i-1)*col+j-1,false), mkLit((i-1)*col+j+1,false), mkLit(i*col+j-1, false), mkLit(i*col+j+1, false), mkLit((i+1)*col+j, true), mkLit((i+1)*col+j-1, true), mkLit((i+1)*col+j+1, true)});
            }
        }
    }
    // 4 edges

    //Preservation(0,0) = C(3,2) = 3!/2! = 3
    addLiterals(maxsat_formula, {mkLit(0, true), mkLit(1, true), mkLit(col, true), mkLit(col+1, false)});
    addLiterals(maxsat_formula, {mkLit(0, true), mkLit(1, true), mkLit(col, false), mkLit(col+1, true)});
    addLiterals(maxsat_formula, {mkLit(0, true), mkLit(1, false), mkLit(col, true), mkLit(col+1, true)});
    //Life(0,0) = C(3,3) = 3!/3! = 1
    addLiterals(maxsat_formula, {mkLit(1, true), mkLit(col, true), mkLit(col+1, true)});

    //Preservation(0,col-1) = C(3,2) = 3!/2! = 3
    addLiterals(maxsat_formula, {mkLit(col-1, true), mkLit(col-2, true), mkLit(2*col-1, true), mkLit(2*col-2, false)});
    addLiterals(maxsat_formula, {mkLit(col-1, true), mkLit(col-2, true), mkLit(2*col-1, false), mkLit(2*col-2, true)});
    addLiterals(maxsat_formula, {mkLit(col-1, true), mkLit(col-2, false), mkLit(2*col-1, true), mkLit(2*col-2, true)});
    //Life(0,col-1) = C(3,3) = 3!/3! = 1
    addLiterals(maxsat_formula, {mkLit(col-2, true), mkLit(2*col-1, true), mkLit(2*col-2, true)});

    //Preservation(lin-1,0) = C(3,2) = 3!/2! = 3
    addLiterals(maxsat_formula, {mkLit((lin-1)*col, true), mkLit((lin-1)*col+1, true), mkLit((lin-2)*col, true), mkLit((lin-2)*col+1, false)});
    addLiterals(maxsat_formula, {mkLit((lin-1)*col, true), mkLit((lin-1)*col+1, true), mkLit((lin-2)*col, false), mkLit((lin-2)*col+1, true)});
    addLiterals(maxsat_formula, {mkLit((lin-1)*col, true), mkLit((lin-1)*col+1, false), mkLit((lin-2)*col, true), mkLit((lin-2)*col+1, true)});
    //Life(lin-1,0) = C(3,3) = 3!/3! = 1
    addLiterals(maxsat_formula, {mkLit((lin-1)*col+1, true), mkLit((lin-2)*col, true), mkLit((lin-2)*col+1, true)});

    //Preservation(lin-1,col-1) = C(3,2) = 3!/2! = 3
    addLiterals(maxsat_formula, {mkLit(lin*col-1, true), mkLit(lin*col-2, true), mkLit((lin-1)*col-1, true), mkLit((lin-1)*col-2, false)});
    addLiterals(maxsat_formula, {mkLit(lin*col-1, true), mkLit(lin*col-2, true), mkLit((lin-1)*col-1, false), mkLit((lin-1)*col-2, true)});
    addLiterals(maxsat_formula, {mkLit(lin*col-1, true), mkLit(lin*col-2, false), mkLit((lin-1)*col-1, true), mkLit((lin-1)*col-2, true)});
    //Life(lin-1,col-1) = C(3,3) = 3!/3! = 1
    addLiterals(maxsat_formula, {mkLit(lin*col-2, true), mkLit((lin-1)*col-1, true), mkLit((lin-1)*col-2, true)});

    //First and last line
    for (int j = 0; j < col-1; j++){
      //Preservation(0,j) = C(5,2) = 5!/2!3! = 10
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, true), mkLit(j+1, true), mkLit(col+j-1, false), mkLit(col+j, false), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, true), mkLit(j+1, false), mkLit(col+j-1, true), mkLit(col+j, false), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, true), mkLit(j+1, false), mkLit(col+j-1, false), mkLit(col+j, true), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, true), mkLit(j+1, false), mkLit(col+j-1, false), mkLit(col+j, false), mkLit(col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, false), mkLit(j+1, true), mkLit(col+j-1, true), mkLit(col+j, false), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, false), mkLit(j+1, true), mkLit(col+j-1, false), mkLit(col+j, true), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, false), mkLit(j+1, true), mkLit(col+j-1, false), mkLit(col+j, false), mkLit(col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, false), mkLit(j+1, false), mkLit(col+j-1, true), mkLit(col+j, true), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, false), mkLit(j+1, false), mkLit(col+j-1, true), mkLit(col+j, false), mkLit(col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit(j, true), mkLit(j-1, false), mkLit(j+1, false), mkLit(col+j-1, false), mkLit(col+j, true), mkLit(col+j+1, true)});
      //Life(0,j) = C(5,3) = 5!/3!2! = 10
      addLiterals(maxsat_formula, {mkLit(j-1, true), mkLit(j+1, true), mkLit(col+j-1, true), mkLit(col+j, false), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j-1, true), mkLit(j+1, true), mkLit(col+j-1, false), mkLit(col+j, true), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j-1, true), mkLit(j+1, true), mkLit(col+j-1, false), mkLit(col+j, false), mkLit(col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit(j-1, true), mkLit(j+1, false), mkLit(col+j-1, true), mkLit(col+j, true), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j-1, true), mkLit(j+1, false), mkLit(col+j-1, true), mkLit(col+j, false), mkLit(col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit(j-1, true), mkLit(j+1, false), mkLit(col+j-1, false), mkLit(col+j, true), mkLit(col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit(j-1, false), mkLit(j+1, true), mkLit(col+j-1, true), mkLit(col+j, true), mkLit(col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit(j-1, false), mkLit(j+1, true), mkLit(col+j-1, true), mkLit(col+j, false), mkLit(col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit(j-1, false), mkLit(j+1, true), mkLit(col+j-1, false), mkLit(col+j, true), mkLit(col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit(j-1, false), mkLit(j+1, false), mkLit(col+j-1, true), mkLit(col+j, true), mkLit(col+j+1, true)});

       //Preservation(lin-1,j) = C(5,2) = 5!/2!3! = 10
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j, true), mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, true)});
      //Life(lin-1,j) = C(5,3) = 5!/3!2! = 10
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, true), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, false)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, false), mkLit((lin-2)*col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, true), mkLit((lin-2)*col+j-1, false), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, true)});
      addLiterals(maxsat_formula, {mkLit((lin-1)*col+j-1, false), mkLit((lin-1)*col+j+1, false), mkLit((lin-2)*col+j-1, true), mkLit((lin-2)*col+j, true), mkLit((lin-2)*col+j+1, true)}); 
    }

    //Right and left columns
    for (int i = 1; i < lin-1; i++){
      //Preservation(i,0) = C(5,2) = 5!/2!3! = 10
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, true), mkLit((i-1)*col+1, true), mkLit(i*col+1, false), mkLit((i+1)*col, false), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, true), mkLit((i-1)*col+1, false), mkLit(i*col+1, true), mkLit((i+1)*col, false), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, true), mkLit((i-1)*col+1, false), mkLit(i*col+1, false), mkLit((i+1)*col, true), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, true), mkLit((i-1)*col+1, false), mkLit(i*col+1, false), mkLit((i+1)*col, false), mkLit((i+1)*col+1, true)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, false), mkLit((i-1)*col+1, true), mkLit(i*col+1, true), mkLit((i+1)*col, false), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, false), mkLit((i-1)*col+1, true), mkLit(i*col+1, false), mkLit((i+1)*col, true), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, false), mkLit((i-1)*col+1, true), mkLit(i*col+1, false), mkLit((i+1)*col, false), mkLit((i+1)*col+1, true)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, false), mkLit((i-1)*col+1, false), mkLit(i*col+1, true), mkLit((i+1)*col, true), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, false), mkLit((i-1)*col+1, false), mkLit(i*col+1, true), mkLit((i+1)*col, false), mkLit((i+1)*col+1, true)});
      addLiterals(maxsat_formula, {mkLit(i*col, true), mkLit((i-1)*col, false), mkLit((i-1)*col+1, false), mkLit(i*col+1, false), mkLit((i+1)*col, true), mkLit((i+1)*col+1, true)});
      //Life(i,0) = C(5,3) = 5!/3!2! = 10
      addLiterals(maxsat_formula, {mkLit((i-1)*col, true), mkLit((i-1)*col+1, true), mkLit(i*col+1, true), mkLit((i+1)*col, false), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, true), mkLit((i-1)*col+1, true), mkLit(i*col+1, false), mkLit((i+1)*col, true), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, true), mkLit((i-1)*col+1, true), mkLit(i*col+1, false), mkLit((i+1)*col, false), mkLit((i+1)*col+1, true)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, true), mkLit((i-1)*col+1, false), mkLit(i*col+1, true), mkLit((i+1)*col, true), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, true), mkLit((i-1)*col+1, false), mkLit(i*col+1, true), mkLit((i+1)*col, false), mkLit((i+1)*col+1, true)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, true), mkLit((i-1)*col+1, false), mkLit(i*col+1, false), mkLit((i+1)*col, true), mkLit((i+1)*col+1, true)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, false), mkLit((i-1)*col+1, true), mkLit(i*col+1, true), mkLit((i+1)*col, true), mkLit((i+1)*col+1, false)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, false), mkLit((i-1)*col+1, true), mkLit(i*col+1, true), mkLit((i+1)*col, false), mkLit((i+1)*col+1, true)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, false), mkLit((i-1)*col+1, true), mkLit(i*col+1, false), mkLit((i+1)*col, true), mkLit((i+1)*col+1, true)});
      addLiterals(maxsat_formula, {mkLit((i-1)*col, false), mkLit((i-1)*col+1, false), mkLit(i*col+1, true), mkLit((i+1)*col, true), mkLit((i+1)*col+1, true)});

       //Preservation(i,col-1)) = C(5,2) = 5!/2!3! = 10
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, true), mkLit(i*col-2, true), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, true), mkLit(i*col-2, false), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, true), mkLit(i*col-2, false), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, true), mkLit(i*col-2, false), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, true)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, false), mkLit(i*col-2, true), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, false), mkLit(i*col-2, true), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, false), mkLit(i*col-2, true), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, true)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, false), mkLit(i*col-2, false), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, false), mkLit(i*col-2, false), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, true)});
      addLiterals(maxsat_formula, {mkLit((i+1)*col-1, true), mkLit(i*col-1, false), mkLit(i*col-2, false), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, true)});
      //Life(i,col-1) = C(5,3) = 5!/3!2! = 10
      addLiterals(maxsat_formula, {mkLit(i*col-1, true), mkLit(i*col-2, true), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, true), mkLit(i*col-2, true), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, true), mkLit(i*col-2, true), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, true)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, true), mkLit(i*col-2, false), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, true), mkLit(i*col-2, false), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, true)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, true), mkLit(i*col-2, false), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, true)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, false), mkLit(i*col-2, true), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, false)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, false), mkLit(i*col-2, true), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, false), mkLit((i+2)*col-1, true)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, false), mkLit(i*col-2, true), mkLit((i+1)*col-2, false), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, true)});
      addLiterals(maxsat_formula, {mkLit(i*col-1, false), mkLit(i*col-2, false), mkLit((i+1)*col-2, true), mkLit((i+2)*col-2, true), mkLit((i+2)*col-1, true)}); 
    }
    for (int i = 0; i < lin*col; i++){
      vec<Lit> clause;
      clause.push(mkLit(i, true));
      maxsat_formula->addSoftClause(1, clause);
    }
    
    if (S->getMaxSATFormula() == NULL)
      S->loadFormula(maxsat_formula);
    S->setInitialTime(initial_time);
    mxsolver = S;
    mxsolver->setPrint(true);
    int ret = (int)mxsolver->search();
    if (mxsolver->getValue(1) == 0){ //Unsat
      delete S;
      return 0;
    }
    for (int i = 0; i < lin; i++){
      for (int j = 0; j < col; j++){
        if (mxsolver->getValue(i*col+j) < 1)
          printf ("0 ");
        else
          printf ("1 ");
      }
      printf ("\n");
    }
    delete S;
    return 0;
  } 
