/*
 * Developped _by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
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
    std::shared_ptr<Professional> pro;
    std::shared_ptr<StudentGroup> group;
    std::shared_ptr<TimeSlot> slot;
    Assignation(std::shared_ptr<Professional> p_pro, std::shared_ptr<StudentGroup> p_group,
                std::shared_ptr<TimeSlot> p_slot);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const Assignation& r_assign) { return r_assign.print(os); };

// Hold and display a Solution of the problem
struct Solution {
    std::vector<std::unique_ptr<Assignation>> assignations;
    Solution(std::vector<std::unique_ptr<Assignation>>& assignations);
    std::ostream& print(std::ostream& os = std::cout) const;
    void writeDays(std::unique_ptr<Data>& p_data, OpenXLSX::XLWorksheet& r_sheet, int row_offset,
                   int start_date_col, int start_day, int end_day);
    void writeSlots(std::unique_ptr<Data>& p_data, OpenXLSX::XLWorksheet& r_sheet, int row_offset);
    void writeAssignations(std::unique_ptr<Data>& p_data, OpenXLSX::XLWorksheet& r_sheet, int row_dff,
                           int start_date_col, int start_day, int end_day);
    void writeXLS(std::unique_ptr<Data>& p_data); // Write solution to XLSfile
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
    std::unique_ptr<Data>& p_data;
    int num_assign;
    std::vector<int> num_assign_by_slot;
    std::vector<int> num_assign_by_day;
    std::vector<int> num_assign_by_pro;
    std::vector<int> num_assign_by_group;
    float std_assign_by_slot;
    float std_assign_by_day;
    float std_assign_by_pro;
    float std_assign_by_group;
    SolutionEvaluation(std::unique_ptr<Data>& p_data, int num_assign,
                       std::vector<int>& r_num_assign_by_slot, std::vector<int>& r_num_assign_by_day,
                       std::vector<int>& r_num_assign_by_pro, std::vector<int>& r_num_assign_by_group,
                       float std_assign_by_slot, float std_assign_by_day, float std_assign_by_pro,
                       float std_assign_by_group);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const SolutionEvaluation& sol_eval) {
    return sol_eval.print(os);
};

// Return the evaluation of the quality of a solution
SolutionEvaluation* evaluate(std::unique_ptr<Data>& data, std::unique_ptr<Solution>& sol);