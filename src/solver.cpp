/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solver.cpp
 */

#include <iostream>
#include <fstream>

#include "ortools/linear_solver/linear_solver.h"

#include "data.h"
#include "heuristic.h"
#include "solution.h"
#include "solver.h"

using namespace std;
using namespace operations_research;

void AlgorithmStrategy::setAlgorithm(int type)
{
    if (type == Heuristic)
        algo_ = new AlgorithmHeuristic();
}

Solution* AlgorithmStrategy::solve(Data* data)
{
    return algo_->solve(data);
}

Solution* AlgorithmHeuristic::solve(Data* data)
{
    HeurNode* bestNode = pseudoGenetic(data);
    Solution* solution = buildSolution(data, bestNode);
    return solution;
}

