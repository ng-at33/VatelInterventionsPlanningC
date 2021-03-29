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

Solution::Solution(vector<Assignation *>& assignations) : assignations(assignations) {};

Solution* buildSolution(Data& data, VatelModel& model) {
    vector<Assignation *> assignations {};
    for (auto& xVarPair : model.xVarMap) {
        auto& xVarIdx = xVarPair.first;
        auto& xVar = xVarPair.second;
        if (xVar->solution_value() == 1.0) {
            auto proIdx = get<0>(xVarIdx);
            auto slotIdx = get<1>(xVarIdx);
            Assignation* assignation = new Assignation();
            assignation->pro = data.professionals[proIdx];
            assignation->slot = data.slots[slotIdx];
            assignations.push_back(assignation);
        }
    }
    Solution* solution = new Solution(assignations);
    return solution;
};

Solution* buildSolution(Data& data, HeurNode& node) {
    vector<Assignation *> assignations {};
    auto slotIdx = 0;
    for (auto& slot: node.slots) {
        for (auto& pair: slot) {
            Assignation* assignation = new Assignation();
            assignation->pro = pair.first;
            assignation->group = pair.second;
            assignation->slot = data.slots[slotIdx];
            assignations.push_back(assignation);
        }
        slotIdx++;
    }
    Solution* solution = new Solution(assignations);
    return solution;
};

ostream& Assignation::print(ostream& os) const {
    os << "Assignation(" << pro->name << " - " << group->name << " @ " << slot->name << ")" << endl;
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

    auto rowOff = 1;
    auto startDateCol = 1;
    auto startSlotRow = 0 + rowOff;
    // Writing days
    for (auto iDay = 0; iDay < data.config.days.size(); iDay++) {
        auto dayStr = data.config.days[iDay];
        sheet->writeStr(startSlotRow, startDateCol + iDay, dayStr.c_str());
    }
    auto startSlotCol = 0;
    auto rowSlotOffset = 1 + rowOff;
    // Writing slots
    for (auto iSlot = 0; iSlot < data.config.slots.size(); iSlot++) {
        auto slotStr = data.config.slots[iSlot];
        sheet->writeStr(iSlot + rowSlotOffset, startSlotCol, slotStr.c_str());
    }
    auto rowAssOff = 1 + rowOff;
    // Writing assignations
    for (auto d = 0; d < data.config.nbDays; d++) {
        for (auto s = 0; s < data.config.nbSlotsByDay; s++) {
            auto row = s + rowAssOff;
            auto col = startDateCol + d;
            string cellContent = "";
            // Filtering affecations that are on this day/slot
            for (auto& af : this->assignations) {
                if (af->slot->day == d && af->slot->slotOfDay == s) {
                    cellContent += "\n" + af->pro->name;
                }
            }
            sheet->writeStr(row, col, cellContent.c_str());
        }
    }
    book->save("planning.xls");
    book->release();
}

bool validateSolution(Data& data, Solution& sol) {
    auto isSolValid = true;
    Assignation* assignation = new Assignation();
    vector<int> nbAssByPr(data.dimensions.numPros, 0);
    vector<int> nbAssBySl(data.dimensions.numSlots, 0);
    // Computing number of interventions by professional and slot
    for (auto& af : sol.assignations) {
        // Checking that the professional was available on this time slot
        if (find(af->pro->slots.begin(), af->pro->slots.end(), af->slot) == af->pro->slots.end()) {
            cout << "ERROR: " << af->pro->name << " not available on time slot " <<
                af->slot->name << endl;
        }
        nbAssByPr[af->pro->idx]++;
        nbAssBySl[af->slot->idx]++;
    }
    // Checking that no professional has exceeded its max number of assignations
    for (auto& pr : data.professionals) {
        if (nbAssByPr[pr->idx] >= 4) {
            cout << "ERROR: " << pr->name << " found assigned more " << "than 3 times" << endl;
        }
    }
    // Checking that no slot has exceeded its max number of assignations
    for (auto& sl : data.slots) {
        if (nbAssBySl[sl->idx] >= 4) {
            cout << "ERROR: " << sl->name << " found assigned more " << "than 3 times" << endl;
        }
    }
    // Checking that all professionals are not scheduled more than once per time slot
    for (auto& slot : data.slots) {
        vector<Professional *> prosInSlot {};
        for (auto& af : sol.assignations) {
            if (af->slot == slot) {
                if (find(prosInSlot.begin(), prosInSlot.end(), af->pro) != prosInSlot.end()) {
                    cout << "ERROR: " << (*af->pro).name << " found assigned in slot "
                        << *slot << " more than once" << endl;
                    isSolValid = false;
                }
                prosInSlot.push_back(af->pro);
            }
        }
    }
    // Checking that all students groups are not scheduled more than once per time slot
    for (auto& slot : data.slots) {
        vector<StudentGroup *> sgInSlot {};
        for (auto& af : sol.assignations) {
            if (af->slot == slot) {
                if (find(sgInSlot.begin(), sgInSlot.end(), af->group) != sgInSlot.end()) {
                    cout << "ERROR: " << (*af->group).name << " found assigned in slot "
                        << *slot << " more than once" << endl;
                    isSolValid = false;
                }
                sgInSlot.push_back(af->group);
            }
        }
    }
    return isSolValid;
};