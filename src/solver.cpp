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

Solution* AlgorithmStrategy::solve(Data* pData)
{
    return algo_->solve(pData);
}

Solution* AlgorithmHeuristic::solve(Data* pData)
{
    HeurNode* pBestNode = pseudoGenetic(pData);
    Solution* pSolution = buildSolution(pData, pBestNode);
    return pSolution;
}

