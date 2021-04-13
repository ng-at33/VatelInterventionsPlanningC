/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * main.cpp: test this small program
 */

#include <iostream>
#include <fstream>
#include <string>

#include "data.h"
#include "solution.h"
#include "solver.h"

using namespace std;

int main(int argc, char** argv) {
    string fn = "data/doodle_new.xlsx";
    Data* data = readXLS(fn);
    // Data* data = generateData(50, 10, 1.0, 0.5);
    cout << *data << endl;
    AlgorithmStrategy algo;
    // 0 : MIP (Obsolete), 1 : Heuristic
    algo.setAlgorithm(1);
    Solution* sol = algo.solve(data);
    sol->print(cout);
    auto isSolValid = validateSolution(data, sol);
    SolutionEvaluation* solEval = evaluate(data, sol);
    sol->writeXLS(data);
    cout << *solEval << endl;

    return EXIT_SUCCESS;
}