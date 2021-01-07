/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solution.cpp
 */

#include <iostream>
#include <string>
#include <vector>

#include "ortools/linear_solver/linear_solver.h"

#include "solution.h"

using namespace std;
using namespace operations_research;

Solution* buildSolution(Data& data, VatelModel& model) {
    Solution* solution = new Solution();
    std::vector<Assignation *> assignations;
    for (auto& xVarPair : model.xVarMap) {
        auto& xVarIdx = xVarPair.first;
        auto proIdx = get<0>(xVarIdx);
        auto slotIdx = get<1>(xVarIdx);
        Assignation* assignation = new Assignation();
        assignation->pro = data.professionals[proIdx];
        assignation->slot = data.slots[slotIdx];
        assignations.push_back(assignation);
    }
    solution->assignations = assignations;
    return solution;
};

ostream& Assignation::print(ostream& os) const {
    os << "Assignation(";
    os << pro->name << " @ ";
    os << slot->name;
    os << ")" << endl;
    return os;
};

ostream& Solution::print(ostream& os) const {
    os << "Solution(Assignations" << endl;
    for (auto& as : assignations) as->print(os);
    os << ")" << endl;
    return os;
};

bool validateSolution(Data& data, Solution& sol) {    
    // Checking that all professionals are not scheduled more than once per time slot

};