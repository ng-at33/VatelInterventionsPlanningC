/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solution.h: define and builds a structure holding a solution,
 * as well as functions to evaluation its quality and display it
 */

#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "ortools/linear_solver/linear_solver.h"

#include "data.h"
#include "model.h"
#include "heuristic.h"

// Hold the information about one intervention
struct Assignation {
    Professional* pro;
    StudentGroup* group;
    TimeSlot* slot;
    Assignation(Professional* pro, StudentGroup* group, TimeSlot* slot);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Assignation& as) {
    return as.print(os);
};

// Hold and display a Solution of the problem
struct Solution {
    std::vector<Assignation *> assignations;
    Solution(std::vector<Assignation *>& assignations);
    std::ostream& print(std::ostream& os = std::cout) const;
    void writeDays(Data* data, OpenXLSX::XLWorksheet& sheet, int rowOffset, int startDateCol, int startDay, int endDay);
    void writeSlots(Data* data, OpenXLSX::XLWorksheet& sheet, int rowOffset);
    void writeAssignations(Data* data, OpenXLSX::XLWorksheet& sheet, int rowOff, int startDateCol,
        int startDay, int endDay);
    void writeXLS(Data* data); // Write solution to XLSfile
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Solution& sol) {
    return sol.print(os);
};

// TODO: define a generic data structure to hold algorithms solutions
// Build a Solution from variables values
Solution* buildSolution(Data* data, VatelModel* model);
// Build a solution from heuristic node
Solution* buildSolution(Data* data, HeurNode* model);
// Produce a SolutionProfiling to validate and also evaluate the quality of a solution 
// SolutionProfiling* evaluteSolution(Data& data, Solution& sol);

// Validate that a Solution is valid regarding all constraints of the problem
bool validateSolution(Data* data, Solution* sol);

// Hold the result of the evulation of a solution
struct SolutionEvaluation {
    int numAssign;
    float stdevAssignBySlot;
    float stdevAssignByDay;
    float stdevAssignByPro;
    float stdevAssignByGroup;
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const SolutionEvaluation& solEval) {
    return solEval.print(os);
};

// Return the evaluation of the quality of a solution
SolutionEvaluation* evaluate(Data* data, Solution* sol);