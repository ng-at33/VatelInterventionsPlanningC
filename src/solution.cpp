/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * solution.cpp
 */

#include <iostream>
#include <string>
#include <vector>

#include <OpenXLSX.hpp>


#include "constants.h"
#include "solution.h"
#include "utils.h"

using namespace std;
using namespace OpenXLSX;

Assignation::Assignation(Professional* pPro, StudentGroup* pGroup, TimeSlot* pSlot) :
    pro(pPro), group(pGroup),  slot(pSlot)
    {};

Solution::Solution(vector<Assignation *>& rAssignations) : assignations(rAssignations) {};

unique_ptr<Solution> buildSolution(unique_ptr<Data>& pData, unique_ptr<HeurNode>& node) {
    vector<Assignation *> assignations {};
    auto slotIdx = 0;
    for (auto const& rSlot: node->slots) {
        for (auto const& rPair: rSlot) {
            Assignation* assignation = new Assignation(rPair.first, rPair.second, pData->slots[slotIdx]);
            assignations.push_back(assignation);
        }
        slotIdx++;
    }
    auto pSolution = make_unique<Solution>(assignations);
    return pSolution;
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

void Solution::writeDays(unique_ptr<Data>& pData, XLWorksheet& rSheet, int rowOff, int startDateCol,
        int startDay, int endDay) {
    auto startSlotRow = 0 + rowOff;
    // Writing days
    for (auto iDay = 0; iDay < endDay - startDay + 1; iDay++) {
        auto dayIdx = iDay + startDay;
        auto dayStr = pData->config.days[dayIdx];
        auto cellDay = rSheet.cell(XLCellReference(startSlotRow + 1, startDateCol + iDay + 1));
        cellDay.value() = dayStr.c_str();
    }
}

void Solution::writeSlots(unique_ptr<Data>& pData, XLWorksheet& rSheet, int rowOff) {
    auto startSlotCol = 0;
    auto rowSlotOffset = 1 + rowOff;
    // Writing slots
    for (unsigned long iSlot = 0; iSlot < pData->config.slots.size(); iSlot++) {
        auto slotStr = pData->config.slots[iSlot];
        auto cellSlot = rSheet.cell(XLCellReference(iSlot + rowSlotOffset + 1, startSlotCol + 1));
        cellSlot.value() = slotStr.c_str();
    }
}

void Solution::writeAssignations(unique_ptr<Data>& pData, XLWorksheet& rSheet, int rowOff, int startDateCol,
        int startDay, int endDay) {
    auto rowAssOff = 1 + rowOff;
    // Writing assignations
    for (auto d = startDay; d < endDay + 1; d++) {
        for (auto s = 0; s < pData->config.nbSlotsByDay; s++) {
            auto row = s + rowAssOff;
            auto col = startDateCol + d - startDay;
            string cellContent = "";
            // Filtering affecations that are on this day/slot
            for (auto& rAssign : this->assignations) {
                if (rAssign->slot->day == d && rAssign->slot->slotOfDay == s) {
                    cellContent += rAssign->group->name + " - " + rAssign->pro->name + "\n";
                }
            }
            auto cellAssign = rSheet.cell(XLCellReference(row + 1, col + 1));
            cellAssign.value() = cellContent.c_str();
        }
    }
}

void Solution::writeXLS(unique_ptr<Data>& pData) {
    XLDocument doc;
    doc.create("./Planning.xls");
    doc.workbook().addWorksheet("Planning");
    auto sheet = doc.workbook().worksheet("Planning");
    auto startDay = 0;
    auto endDay = 4;
    auto rowOff = 0;
    auto startDateCol = 1;
    writeDays(pData, sheet, rowOff, startDateCol, startDay, endDay);
    writeSlots(pData, sheet, rowOff);
    writeAssignations(pData, sheet, rowOff, startDateCol, startDay, endDay);
    rowOff = 6;
    startDay = 5;
    endDay = pData->config.nbDays;
    writeDays(pData, sheet, rowOff, startDateCol, startDay, endDay);
    writeSlots(pData, sheet, rowOff);
    writeAssignations(pData, sheet, rowOff, startDateCol, startDay, endDay);
    doc.save();
    doc.close();
}

bool validateSolution(unique_ptr<Data>& pData, unique_ptr<Solution>& pSol) {
    auto isSolValid = true;
    set<pair<Professional*, StudentGroup*> > assignations; // Used to check if a <pro,group> is not assigned more than once
    for (auto const& rAssign : pSol->assignations) {
        for (auto const& rOAssign : pSol->assignations) {
            if (rAssign != rOAssign) {
                if (rAssign->pro == rOAssign->pro && rAssign->group == rOAssign->group) {
                    cout << "ERROR: " << rAssign->pro->name << " and " << rAssign->group->name
                        << " assigned on slots " << *rAssign->slot << " and " << *rOAssign->slot << endl;
                }
            }
        }
    }
    vector<int> nbAssByPr(pData->dimensions.numPros, 0);
    vector<int> nbAssBySl(pData->dimensions.numSlots, 0);
    // Computing number of interventions by professional and slot
    for (auto const& rAssign : pSol->assignations) {
        // Checking that the professional was available on this time slot
        if (find(rAssign->pro->slots.begin(), rAssign->pro->slots.end(), rAssign->slot) == rAssign->pro->slots.end()) {
            cout << "ERROR: " << rAssign->pro->name << " not available on time slot " <<
                rAssign->slot->name << endl;
        }
        // Checking that the professional and the students group are compatible
        if (find(rAssign->pro->groups.begin(), rAssign->pro->groups.end(), rAssign->group)
                == rAssign->pro->groups.end()) {
            cout << "ERROR: " << rAssign->pro->name << " and " << rAssign->group->name <<
                " are not compatible" << endl;
        }
        nbAssByPr[rAssign->pro->idx]++;
        nbAssBySl[rAssign->slot->idx]++;
    }
    // Checking that no professional has exceeded its max number of assignations
    for (auto const& rPro : pData->professionals) {
        if (nbAssByPr[rPro->idx] > G_MAX_NUMBER_INTERV_PRO) {
            cout << "ERROR: " << rPro->name << " found assigned more " << "than " 
                << G_MAX_NUMBER_INTERV_PRO << " times" << endl;
        }
    }
    // Checking that no slot has exceeded its max number of assignations
    for (auto const& rSlot : pData->slots) {
        if (nbAssBySl[rSlot->idx] > 3) {
            cout << "ERROR: " << rSlot->name << " found assigned more " << "than "
                << G_MAX_NUMBER_INTERV_SLOT << " times" << endl;
        }
    }
    // Checking that all professionals are not scheduled more than once per time slot
    for (auto const& rSlot : pData->slots) {
        vector<Professional *> prosInSlot {};
        for (auto const& rAssign : pSol->assignations) {
            if (rAssign->slot == rSlot) {
                if (find(prosInSlot.begin(), prosInSlot.end(), rAssign->pro) != prosInSlot.end()) {
                    cout << "ERROR: " << (*rAssign->pro).name << " found assigned in slot "
                        << *rSlot << " more than once" << endl;
                    isSolValid = false;
                }
                prosInSlot.push_back(rAssign->pro);
            }
        }
    }
    // Checking that all students groups are not scheduled more than once per time slot
    for (auto const& rSlot : pData->slots) {
        vector<StudentGroup *> sgInSlot {};
        for (auto const& rAssign : pSol->assignations) {
            if (rAssign->slot == rSlot) {
                if (find(sgInSlot.begin(), sgInSlot.end(), rAssign->group) != sgInSlot.end()) {
                    cout << "ERROR: " << (*rAssign->group).name << " found assigned in slot "
                        << *rSlot << " more than once" << endl;
                    isSolValid = false;
                }
                sgInSlot.push_back(rAssign->group);
            }
        }
    }
    return isSolValid;
};

SolutionEvaluation::SolutionEvaluation(unique_ptr<Data>& pData, int numAssign, vector<int>& rNumAssignBySlot,
        vector<int>& rNumAssignByDay, vector<int>& rNumAssignByPro,
        vector<int>& rNumAssignByGroup, float stdevAssignBySlot, float stdevAssignByDay,
        float stdevAssignByPro, float stdevAssignByGroup) : pData(pData), numAssign(numAssign),
        numAssignBySlot(rNumAssignBySlot), numAssignByDay(rNumAssignByDay),
        numAssignByPro(rNumAssignByPro), numAssignByGroup(rNumAssignByGroup),
        stdevAssignBySlot(stdevAssignBySlot), stdevAssignByDay(stdevAssignByDay),
        stdevAssignByPro(stdevAssignByPro), stdevAssignByGroup(stdevAssignByGroup) {}

ostream& SolutionEvaluation::print(ostream& os) const {
    os << "SolutionEvaluation(Number of assignations : " << numAssign << endl;
    os << "Standard deviation of the number of assignations by slot " << stdevAssignBySlot << endl;
    os << "Standard deviation of the assignations by day " << stdevAssignByDay << endl;
    os << "Standard deviation of the assignations by professional " << stdevAssignByPro << endl;
    os << "Standard deviation of the assignations by group " << stdevAssignByGroup << endl;
    os << "Number of assignations by pro :" << endl;
    for (auto const& rPro : pData->professionals) {
        os << "    " << rPro->name << " : " << numAssignByPro[rPro->idx] << " / "
            << rPro->slots.size() << endl;
    }
    return os;
};

SolutionEvaluation* evaluate(unique_ptr<Data>& pData, unique_ptr<Solution>& pSol) {
    int numAssign = pSol->assignations.size();
    vector<int> numAssignBySlot(pData->dimensions.numSlots, 0);
    vector<int> numAssignByDay(pData->config.nbDays, 0);
    vector<int> numAssignByPro(pData->dimensions.numPros, 0);
    vector<int> numAssignByGroup(pData->dimensions.numGroups, 0);
    for (auto const& rAssign : pSol->assignations) {
        numAssignBySlot[rAssign->slot->idx]++;
        numAssignByDay[rAssign->slot->day]++;
        numAssignByPro[rAssign->pro->idx]++;
        numAssignByGroup[rAssign->group->idx]++;
    }
    float stdevAssignBySlot = computeSDVec(numAssignBySlot);
    float stdevAssignByDay = computeSDVec(numAssignByDay);
    float stdevAssignByPro = computeSDVec(numAssignByPro);
    float stdevAssignByGroup = computeSDVec(numAssignByGroup);
    SolutionEvaluation* pSolEval = new SolutionEvaluation(pData, numAssign, numAssignBySlot,
        numAssignByDay, numAssignByPro, numAssignByGroup, stdevAssignBySlot, stdevAssignByDay,
        stdevAssignByPro, stdevAssignByGroup);
    
    return pSolEval;
};