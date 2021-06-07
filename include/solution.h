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
#include "heuristic.h"

// Hold the information about one intervention
struct Assignation {
    Professional* pro;
    StudentGroup* group;
    TimeSlot* slot;
    Assignation(Professional* pPro, StudentGroup* pGroup, TimeSlot* pSlot);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Assignation& rAssign) { return rAssign.print(os); };

// Hold and display a Solution of the problem
struct Solution {
    std::vector<Assignation *> assignations;
    Solution(std::vector<Assignation *>& assignations);
    std::ostream& print(std::ostream& os = std::cout) const;
    void writeDays(std::unique_ptr<Data>& pData, OpenXLSX::XLWorksheet& rSheet, int rowOffset, int startDateCol, int startDay, int endDay);
    void writeSlots(std::unique_ptr<Data>& pData, OpenXLSX::XLWorksheet& rSheet, int rowOffset);
    void writeAssignations(std::unique_ptr<Data>& pData, OpenXLSX::XLWorksheet& rSheet, int rowOff, int startDateCol,
        int startDay, int endDay);
    void writeXLS(std::unique_ptr<Data>& pData); // Write solution to XLSfile
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Solution& sol) {
    return sol.print(os);
};

// TODO: define a generic data structure to hold algorithms solutions
// Build a solution from heuristic node
std::unique_ptr<Solution> buildSolution(std::unique_ptr<Data>& data, std::unique_ptr<HeurNode>& node);
// Produce a SolutionProfiling to validate and also evaluate the quality of a solution 
// SolutionProfiling* evaluteSolution(Data& data, Solution& sol);

// Validate that a Solution is valid regarding all constraints of the problem
bool validateSolution(std::unique_ptr<Data>& data, std::unique_ptr<Solution>& sol);

// Hold the result of the evulation of a solution
struct SolutionEvaluation {
    std::unique_ptr<Data>& pData;
    int numAssign;
    std::vector<int> numAssignBySlot;
    std::vector<int> numAssignByDay;
    std::vector<int> numAssignByPro;
    std::vector<int> numAssignByGroup;
    float stdevAssignBySlot;
    float stdevAssignByDay;
    float stdevAssignByPro;
    float stdevAssignByGroup;
    SolutionEvaluation(std::unique_ptr<Data>& pData, int numAssign, std::vector<int>& rNumAssignBySlot,
        std::vector<int>& rNumAssignByDay, std::vector<int>& rNumAssignByPro,
        std::vector<int>& rNumAssignByGroup, float stdevAssignBySlot, float stdevAssignByDay,
        float stdevAssignByPro, float stdevAssignByGroup);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const SolutionEvaluation& solEval) {
    return solEval.print(os);
};

// Return the evaluation of the quality of a solution
SolutionEvaluation* evaluate(std::unique_ptr<Data>& data, std::unique_ptr<Solution>& sol);