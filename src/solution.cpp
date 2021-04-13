/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solution.cpp
 */

#include <iostream>
#include <string>
#include <vector>

#include "ortools/linear_solver/linear_solver.h"

#include <OpenXLSX.hpp>


#include "solution.h"
#include "utils.h"

using namespace std;
using namespace OpenXLSX;
using namespace operations_research;

Assignation::Assignation(Professional* pro, StudentGroup* group, TimeSlot* slot) :
    pro(pro), group(group),  slot(slot)
    {};

Solution::Solution(vector<Assignation *>& assignations) : assignations(assignations) {};

Solution* buildSolution(Data* data, VatelModel* model) {
    vector<Assignation *> assignations {};
    for (auto& xVarPair : model->xVarMap) {
        auto& xVarIdx = xVarPair.first;
        auto& xVar = xVarPair.second;
        if (xVar->solution_value() == 1.0) {
            auto proIdx = get<0>(xVarIdx);
            auto slotIdx = get<1>(xVarIdx);
            Assignation* assignation = new Assignation(data->professionals[proIdx], data->groups[0],
                data->slots[slotIdx]);
            assignations.push_back(assignation);
        }
    }
    Solution* solution = new Solution(assignations);
    return solution;
};

Solution* buildSolution(Data* data, HeurNode* node) {
    vector<Assignation *> assignations {};
    auto slotIdx = 0;
    for (auto& slot: node->slots) {
        for (auto& pair: slot) {
            Assignation* assignation = new Assignation(pair.first, pair.second, data->slots[slotIdx]);
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

void Solution::writeDays(Data* data, XLWorksheet& sheet, int rowOff, int startDateCol, int startDay,
        int endDay) {
    auto startSlotRow = 0 + rowOff;
    // Writing days
    for (auto iDay = 0; iDay < endDay - startDay + 1; iDay++) {
        auto dayIdx = iDay + startDay;
        auto dayStr = data->config.days[dayIdx];
        auto cellDay = sheet.cell(XLCellReference(startSlotRow + 1, startDateCol + iDay + 1));
        cellDay.value() = dayStr.c_str();
    }
}

void Solution::writeSlots(Data* data, XLWorksheet& sheet, int rowOff) {
    auto startSlotCol = 0;
    auto rowSlotOffset = 1 + rowOff;
    // Writing slots
    for (auto iSlot = 0; iSlot < data->config.slots.size(); iSlot++) {
        auto slotStr = data->config.slots[iSlot];
        auto cellSlot = sheet.cell(XLCellReference(iSlot + rowSlotOffset + 1, startSlotCol + 1));
        cellSlot.value() = slotStr.c_str();
    }
}

void Solution::writeAssignations(Data* data, XLWorksheet& sheet, int rowOff, int startDateCol,
        int startDay, int endDay) {
    auto rowAssOff = 1 + rowOff;
    // Writing assignations
    for (auto d = startDay; d < endDay + 1; d++) {
        for (auto s = 0; s < data->config.nbSlotsByDay; s++) {
            auto row = s + rowAssOff;
            auto col = startDateCol + d - startDay;
            string cellContent = "";
            // Filtering affecations that are on this day/slot
            for (auto& af : this->assignations) {
                if (af->slot->day == d && af->slot->slotOfDay == s) {
                    cellContent += af->group->name + " - " + af->pro->name + "\n";
                }
            }
            auto cellAssign = sheet.cell(XLCellReference(row + 1, col + 1));
            cellAssign.value() = cellContent.c_str();
        }
    }
}

void Solution::writeXLS(Data* data) {
    XLDocument doc;
    doc.create("./Planning.xls");
    doc.workbook().addWorksheet("Planning");
    auto sheet = doc.workbook().worksheet("Planning");
    auto startDay = 0;
    auto endDay = 4;
    auto rowOff = 0;
    auto startDateCol = 1;
    writeDays(data, sheet, rowOff, startDateCol, startDay, endDay);
    writeSlots(data, sheet, rowOff);
    writeAssignations(data, sheet, rowOff, startDateCol, startDay, endDay);
    rowOff = 6;
    startDay = 5;
    endDay = data->config.nbDays;
    writeDays(data, sheet, rowOff, startDateCol, startDay, endDay);
    writeSlots(data, sheet, rowOff);
    writeSlots(data, sheet, rowOff);
    writeAssignations(data, sheet, rowOff, startDateCol, startDay, endDay);
    doc.save();
    doc.close();
}

bool validateSolution(Data* data, Solution* sol) {
    auto isSolValid = true;
    set<pair<Professional*, StudentGroup*> > assignations; // Used to check if a <pro,group> is not assigned more than once
    for (auto& af : sol->assignations) {
        for (auto& oaf : sol->assignations) {
            if (af != oaf) {
                if (af->pro == oaf->pro && af->group == oaf->group) {
                    cout << "ERROR: " << af->pro->name << " and " << af->group->name
                        << " assigned on slots " << *af->slot << " and " << *oaf->slot << endl;
                }
            }
        }
    }
    vector<int> nbAssByPr(data->dimensions.numPros, 0);
    vector<int> nbAssBySl(data->dimensions.numSlots, 0);
    // Computing number of interventions by professional and slot
    for (auto& af : sol->assignations) {
        // Checking that the professional was available on this time slot
        if (find(af->pro->slots.begin(), af->pro->slots.end(), af->slot) == af->pro->slots.end()) {
            cout << "ERROR: " << af->pro->name << " not available on time slot " <<
                af->slot->name << endl;
        }
        // Checking that the professional and the students group are compatible
        if (find(af->pro->groups.begin(), af->pro->groups.end(), af->group) == af->pro->groups.end()) {
            cout << "ERROR: " << af->pro->name << " and " << af->group->name <<
                " are not compatible" << endl;
        }
        nbAssByPr[af->pro->idx]++;
        nbAssBySl[af->slot->idx]++;
    }
    // Checking that no professional has exceeded its max number of assignations
    for (auto& pr : data->professionals) {
        if (nbAssByPr[pr->idx] >= 4) {
            cout << "ERROR: " << pr->name << " found assigned more " << "than 3 times" << endl;
        }
    }
    // Checking that no slot has exceeded its max number of assignations
    for (auto& sl : data->slots) {
        if (nbAssBySl[sl->idx] >= 4) {
            cout << "ERROR: " << sl->name << " found assigned more " << "than 3 times" << endl;
        }
    }
    // Checking that all professionals are not scheduled more than once per time slot
    for (auto& slot : data->slots) {
        vector<Professional *> prosInSlot {};
        for (auto& af : sol->assignations) {
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
    for (auto& slot : data->slots) {
        vector<StudentGroup *> sgInSlot {};
        for (auto& af : sol->assignations) {
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

ostream& SolutionEvaluation::print(ostream& os) const {
    os << "SolutionEvaluation(Number of assignations : " << numAssign << endl;
    os << "Standard deviation of the number of assignations by slot " << stdevAssignBySlot << endl;
    os << "Standard deviation of the assignations by day " << stdevAssignByDay << endl;
    os << "Standard deviation of the assignations by professional " << stdevAssignByPro << endl;
    os << "Standard deviation of the assignations by group " << stdevAssignByGroup << endl;
    return os;
};

SolutionEvaluation* evaluate(Data* data, Solution* sol) {
    int numAssign = sol->assignations.size();
    vector<float> numAssignBySlot(data->dimensions.numSlots, 0.0);
    vector<float> numAssignByDay(data->config.nbDays, 0.0);
    vector<float> numAssignByPro(data->dimensions.numPros, 0.0);
    vector<float> numAssignByGroup(data->dimensions.numGroups, 0.0);
    for (auto& as : sol->assignations) {
        numAssignBySlot[as->slot->idx]++;
        numAssignByDay[as->slot->day]++;
        numAssignByPro[as->pro->idx]++;
        numAssignByGroup[as->group->idx]++;
    }
    float stdevAssignBySlot = computeSDVec(numAssignBySlot);
    float stdevAssignByDay = computeSDVec(numAssignByDay);
    float stdevAssignByPro = computeSDVec(numAssignByPro);
    float stdevAssignByGroup = computeSDVec(numAssignByGroup);

    SolutionEvaluation* solEval = new SolutionEvaluation();
    solEval->numAssign = numAssign;
    solEval->stdevAssignBySlot = stdevAssignBySlot;
    solEval->stdevAssignByDay = stdevAssignByDay;
    solEval->stdevAssignByPro = stdevAssignByPro;
    solEval->stdevAssignByGroup = stdevAssignByGroup;
    
    return solEval;
};