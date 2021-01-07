/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * model.h: define and builds a data structure holding all the information
 * needed to solve the problem
 */

#pragma once

#include <tuple>

#include "ortools/linear_solver/linear_solver.h"

#include "data.h"

// Structure that holds the variable in a contiguous way
struct VatelModel {
    std::map<std::tuple<int, int>, operations_research::MPVariable*> xVarMap;
};

// Build and return a VatelModel
VatelModel* buildModel(Data& data, operations_research::MPSolver& solver);