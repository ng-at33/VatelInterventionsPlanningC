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
    auto pData = readXLS(fn);
    // auto pData = generateData(50, 100, 1.0, 0.5);
    cout << *pData << endl;
    AlgorithmStrategy algo;
    // 1 : Heuristic
    algo.setAlgorithm(1);
    auto pSol = algo.solve(pData);
    pSol->print(cout);
    auto isSolValid = validateSolution(pData, pSol);
    auto solEval = evaluate(pData, pSol);
    pSol->writeXLS(pData);
    cout << *solEval << endl;

    return EXIT_SUCCESS;
}