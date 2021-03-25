/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solution.h: define and builds a structure holding a solution,
 * as well as functions to display it
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "ortools/linear_solver/linear_solver.h"

#include "data.h"
#include "model.h"
#include "heuristic.h"

// Structure to hold the information about one intervention
struct Assignation {
    Professional* pro;
    TimeSlot* slot;
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Assignation& as) {
    return as.print(os);
};

// Structure to hold and display a Solution of the problem
struct Solution {
    std::vector<Assignation *> assignations;
    std::ostream& print(std::ostream& os = std::cout) const;
    void writeXLS(Data& data); // Write solution to XLSfile
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Solution& sol) {
    return sol.print(os);
};

// // Structure to hold and display a Solution of the problem
// struct SolutionProfiling {
//     std::vector<Assignation *> assignations;
//     std::ostream& print(std::ostream& os = std::cout) const;
// };

// // Overrides cout
// inline std::ostream& operator<<(std::ostream& os, const Solution& sol) {
//     return sol.print(os);
// };

// Build a Solution from variables values
Solution* buildSolution(Data& data, VatelModel& model);
// Build a solution from heuristic node
Solution* buildSolution(Data& data, HeurNode& model);

// Produce a SolutionProfiling to validate and also evaluate the quality of a solution 
// SolutionProfiling* evaluteSolution(Data& data, Solution& sol);

// Validate that a Solution is valid regarding all constraints of the problem
bool validateSolution(Data& data, Solution& sol);
