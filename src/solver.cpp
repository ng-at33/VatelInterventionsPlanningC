/*
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solver.cpp
 */

#include <iostream>
#include <fstream>

#include "data.h"
#include "heuristic.h"
#include "solution.h"
#include "solver.h"

using namespace std;

void AlgorithmStrategy::setAlgorithm(int type)
{
    if (type == Heuristic)
        algo_ = new AlgorithmHeuristic();
}

unique_ptr<Solution> AlgorithmStrategy::solve(unique_ptr<Data>& p_data)
{
    return algo_->solve(p_data);
}

unique_ptr<Solution> AlgorithmHeuristic::solve(unique_ptr<Data>& p_data)
{
    auto pBestNode = pseudoGenetic(p_data);
    auto pSolution = buildSolution(p_data, pBestNode);
    return pSolution;
}

