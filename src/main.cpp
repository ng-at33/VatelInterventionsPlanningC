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
    auto p_data = readXLS(fn);
    // auto p_data = generateData(50, 100, 1.0, 0.5);
    cout << *p_data << endl;
    AlgorithmStrategy algo;
    // 1 : Heuristic
    algo.setAlgorithm(1);
    auto p_sol = algo.solve(p_data);
    p_sol->print(cout);
    auto is_sol_valid = validateSolution(p_data, p_sol);
    auto sol_eval = evaluate(p_data, p_sol);
    p_sol->writeXLS(p_data);
    cout << *sol_eval << endl;

    return EXIT_SUCCESS;
}