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

unique_ptr<Solution> AlgorithmStrategy::solve(unique_ptr<Data>& pData)
{
    return algo_->solve(pData);
}

unique_ptr<Solution> AlgorithmHeuristic::solve(unique_ptr<Data>& pData)
{
    auto pBestNode = pseudoGenetic(pData);
    auto pSolution = buildSolution(pData, pBestNode);
    return pSolution;
}

