/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solver.cpp
 */

#include <iostream>
#include <fstream>

#include "ortools/linear_solver/linear_solver.h"

#include "data.h"
#include "model.h"
#include "heuristic.h"
#include "solution.h"
#include "solver.h"

using namespace std;
using namespace operations_research;

void AlgorithmStrategy::setAlgorithm(int type)
{
    delete algo_;
    if (type == Heuristic)
        algo_ = new AlgorithmHeuristic();
    else // (type == MIP)
        algo_ = new AlgorithmMIP();
}

Solution* AlgorithmStrategy::solve(Data* data)
{
    return algo_->solve(data);
}

Solution* AlgorithmMIP::solve(Data* data)
{
    MPSolver solver("Vatel rostering", MPSolver::GLOP_LINEAR_PROGRAMMING);
    VatelModel* vatelModel = buildModel(data, solver);
    ofstream lpFStream("mip.lp");
    std::string lpString{};
    solver.ExportModelAsLpFormat(false, &lpString);
    lpFStream << lpString << std::endl;
    lpFStream.close();
    Solution* solution;
    const MPSolver::ResultStatus result_status = solver.Solve();
    // Check that the problem has an optimal solution.
    if (result_status != MPSolver::OPTIMAL) {
        LOG(FATAL) << "The problem does not have an optimal solution!";
    }
    else {
        LOG(INFO) << "Success, objective value : " << solver.Objective().Value() << endl;
        solution = buildSolution(data, vatelModel);
    }
    return solution;
}

Solution* AlgorithmHeuristic::solve(Data* data)
{
    HeurNode* firstNode = firstFit(data);
    vector<HeurNode*> nodes { firstNode };
    for (auto i = 0; i < 100; i++) {
        cout << " iteration " << i << endl;
        nodes = iterate(data, nodes);
    }
    cout << " nodes size " << nodes.size() << endl;
    Solution* solution = buildSolution(data, nodes[0]);
    return solution;
}

