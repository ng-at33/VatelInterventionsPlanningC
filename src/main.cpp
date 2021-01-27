/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * main.cpp: test this small program
 */

#include <iostream>
#include <fstream>
#include <string>

#include "ortools/linear_solver/linear_solver.h"
#include "ortools/linear_solver/model_exporter.h"

#include "data.h"
#include "model.h"
#include "solution.h"
#include "solver.h"

using namespace std;

int main(int argc, char** argv) {
    // string fn = "data/doodle_new.xlsx";
    // Data* data = readXLS(fn);
    Data* data = generateData(50, 20, 0.4);
    AlgorithmStrategy algo;
    // 0 : MIP, 1 : Heuristic
    algo.setAlgorithm(0);
    Solution* sol = algo.solve(*data);
    sol->print(cout);
    auto isSolValid = validateSolution(*data, *sol);
    sol->writeXLS(*data);

    return EXIT_SUCCESS;
}