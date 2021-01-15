/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solution.cpp
 */

#include <iostream>
#include <string>
#include <vector>

#include "ortools/linear_solver/linear_solver.h"

#include "libxl.h"

#include "solution.h"

using namespace std;
using namespace libxl;
using namespace operations_research;

Solution* buildSolution(Data& data, VatelModel& model) {
    Solution* solution = new Solution();
    std::vector<Assignation *> assignations {};
    for (auto& xVarPair : model.xVarMap) {
        auto& xVarIdx = xVarPair.first;
        auto& xVar = xVarPair.second;
        if (xVar->solution_value() == 1.0)
        {
            auto proIdx = get<0>(xVarIdx);
            auto slotIdx = get<1>(xVarIdx);
            Assignation* assignation = new Assignation();
            assignation->pro = data.professionals[proIdx];
            assignation->slot = data.slots[slotIdx];
            assignations.push_back(assignation);
        }
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

void Solution::writeXLS(Data& data) {
    Book* book = xlCreateXMLBook();
    Sheet* sheet = book->addSheet("Planning");

    auto startDateCol = 1;
    auto startSlotRow = 1;
    // Writing days
    for (auto iDay = 0; iDay < data.config.days.size(); iDay++) {
        auto dayStr = data.config.days[iDay];
        sheet->writeStr(startSlotRow, startDateCol + iDay, dayStr.c_str());
    }
    auto startSlotCol = 0;
    auto rowSlotOffset = 1;
    // Writing slots
    for (auto iSlot = 0; iSlot < data.config.slots.size(); iSlot++) {
        auto slotStr = data.config.slots[iSlot];
        sheet->writeStr(startSlotRow + iSlot + rowSlotOffset, startSlotCol,
            slotStr.c_str());
    }
    // Writing assignations
    for (auto d = 0; d < data.config.nbDays; d++) {
        for (auto s = 0; s < data.config.nbSlotsByDay; s++) {
            auto row = startSlotRow + s;
            auto col = startDateCol + d;

            // Filtering affecations that are on this day/slot
            for (auto& af : this->assignations) {
                if (af->slot->day == d && af->slot->slotOfDay == s) {
                    sheet->writeStr(row, col, af->pro->name.c_str());
                }
            }
        }
    }
    book->save("planning.xls");
    book->release();
}

bool validateSolution(Data& data, Solution& sol) {    
    // Checking that all professionals are not scheduled more than once per time slot

};