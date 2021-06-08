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

Assignation::Assignation(shared_ptr<Professional> p_pro, shared_ptr<StudentGroup> p_group,
                         shared_ptr<TimeSlot> p_slot)
        : pro(p_pro), group(p_group),  slot(p_slot) {};

Solution::Solution(vector<unique_ptr<Assignation>>& r_assignations)
        : assignations(std::move(r_assignations)) {};

unique_ptr<Solution> buildSolution(unique_ptr<Data>& p_data, unique_ptr<HeurNode>& node) {
    vector<unique_ptr<Assignation>> assignations {};
    auto slot_idx = 0;
    for (auto const& r_slot: node->slots) {
        for (auto const& r_pair: r_slot) {
            auto assignation = make_unique<Assignation>(r_pair.first, r_pair.second,
                                                        p_data->slots[slot_idx]);
            assignations.push_back(std::move(assignation));
        }
        slot_idx++;
    }
    auto p_solution = make_unique<Solution>(assignations);
    return p_solution;
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

void Solution::writeDays(unique_ptr<Data>& p_data, XLWorksheet& r_sheet, int row_off, int start_date_col,
                         int start_day, int end_day) {
    auto start_slot_row = 0 + row_off;
    // Writing days
    for (auto i_day = 0; i_day < end_day - start_day + 1; i_day++) {
        auto day_idx = i_day + start_day;
        auto day_str = p_data->config.days[day_idx];
        auto cell_day = r_sheet.cell(XLCellReference(start_slot_row + 1, start_date_col + i_day + 1));
        cell_day.value() = day_str.c_str();
    }
}

void Solution::writeSlots(unique_ptr<Data>& p_data, XLWorksheet& r_sheet, int row_off) {
    auto start_slot_col = 0;
    auto row_slot_offset = 1 + row_off;
    // Writing slots
    for (unsigned long iSlot = 0; iSlot < p_data->config.slots.size(); iSlot++) {
        auto slot_str = p_data->config.slots[iSlot];
        auto cell_slot = r_sheet.cell(XLCellReference(iSlot + row_slot_offset + 1, start_slot_col + 1));
        cell_slot.value() = slot_str.c_str();
    }
}

void Solution::writeAssignations(unique_ptr<Data>& p_data, XLWorksheet& r_sheet, int row_off,
                                 int start_date_col, int start_day, int end_day) {
    auto row_ass_off = 1 + row_off;
    // Writing assignations
    for (auto d = start_day; d < end_day + 1; d++) {
        for (auto s = 0; s < p_data->config.nb_slots_by_day; s++) {
            auto row = s + row_ass_off;
            auto col = start_date_col + d - start_day;
            string cell_content = "";
            // Filtering affecations that are on this day/slot
            for (auto& r_assign : this->assignations) {
                if (r_assign->slot->day == d && r_assign->slot->slot_of_day == s) {
                    cell_content += r_assign->group->name + " - " + r_assign->pro->name + "\n";
                }
            }
            auto cell_assign = r_sheet.cell(XLCellReference(row + 1, col + 1));
            cell_assign.value() = cell_content.c_str();
        }
    }
}

void Solution::writeXLS(unique_ptr<Data>& p_data) {
    XLDocument doc;
    doc.create("./Planning.xls");
    doc.workbook().addWorksheet("Planning");
    auto sheet = doc.workbook().worksheet("Planning");
    auto start_day = 0;
    auto end_day = 4;
    auto row_off = 0;
    auto start_date_col = 1;
    writeDays(p_data, sheet, row_off, start_date_col, start_day, end_day);
    writeSlots(p_data, sheet, row_off);
    writeAssignations(p_data, sheet, row_off, start_date_col, start_day, end_day);
    row_off = 6;
    start_day = 5;
    end_day = p_data->config.nb_days;
    writeDays(p_data, sheet, row_off, start_date_col, start_day, end_day);
    writeSlots(p_data, sheet, row_off);
    writeAssignations(p_data, sheet, row_off, start_date_col, start_day, end_day);
    doc.save();
    doc.close();
}

bool validateSolution(unique_ptr<Data>& p_data, unique_ptr<Solution>& p_sol) {
    auto is_sol_valid = true;
    set<pair<unique_ptr<Professional>, unique_ptr<StudentGroup>> > assignations; // Used to check if a <pro,group> is not assigned more than once
    for (auto const& r_assign : p_sol->assignations) {
        for (auto const& r_oassign : p_sol->assignations) {
            if (r_assign != r_oassign) {
                if (r_assign->pro == r_oassign->pro && r_assign->group == r_oassign->group) {
                    cout << "ERROR: " << r_assign->pro->name << " and " << r_assign->group->name
                        << " assigned on slots " << *r_assign->slot << " and " << *r_oassign->slot
                        << endl;
                }
            }
        }
    }
    vector<int> nb_ass_by_pr(p_data->dimensions.num_pros, 0);
    vector<int> nb_ass_by_sl(p_data->dimensions.num_slots, 0);
    // Computing number of interventions by professional and slot
    for (auto const& r_assign : p_sol->assignations) {
        // Checking that the professional was available on this time slot
        if (find(r_assign->pro->slots.begin(), r_assign->pro->slots.end(), r_assign->slot) ==
                r_assign->pro->slots.end()) {
            cout << "ERROR: " << r_assign->pro->name << " not available on time slot " <<
                r_assign->slot->name << endl;
        }
        // Checking that the professional and the students group are compatible
        if (find(r_assign->pro->groups.begin(), r_assign->pro->groups.end(), r_assign->group)
                == r_assign->pro->groups.end()) {
            cout << "ERROR: " << r_assign->pro->name << " and " << r_assign->group->name <<
                " are not compatible" << endl;
        }
        nb_ass_by_pr[r_assign->pro->idx]++;
        nb_ass_by_sl[r_assign->slot->idx]++;
    }
    // Checking that no professional has exceeded its max number of assignations
    for (auto const& rPro : p_data->professionals) {
        if (nb_ass_by_pr[rPro->idx] > G_MAX_NUMBER_INTERV_PRO) {
            cout << "ERROR: " << rPro->name << " found assigned more " << "than "
                << G_MAX_NUMBER_INTERV_PRO << " times" << endl;
        }
    }
    // Checking that no slot has exceeded its max number of assignations
    for (auto const& r_slot : p_data->slots) {
        if (nb_ass_by_sl[r_slot->idx] > 3) {
            cout << "ERROR: " << r_slot->name << " found assigned more " << "than "
                << G_MAX_NUMBER_INTERV_SLOT << " times" << endl;
        }
    }
    // Checking that all professionals are not scheduled more than once per time slot
    for (auto const& r_slot : p_data->slots) {
        vector<shared_ptr<Professional>> pros_in_slot {};
        for (auto const& r_assign : p_sol->assignations) {
            if (r_assign->slot == r_slot) {
                if (find(pros_in_slot.begin(), pros_in_slot.end(), r_assign->pro) != pros_in_slot.end()) {
                    cout << "ERROR: " << (*r_assign->pro).name << " found assigned in slot "
                         << *r_slot << " more than once" << endl;
                    is_sol_valid = false;
                }
                pros_in_slot.push_back(r_assign->pro);
            }
        }
    }
    // Checking that all students groups are not scheduled more than once per time slot
    for (auto const& r_slot : p_data->slots) {
        vector<shared_ptr<StudentGroup>> sg_in_slot {};
        for (auto const& r_assign : p_sol->assignations) {
            if (r_assign->slot == r_slot) {
                if (find(sg_in_slot.begin(), sg_in_slot.end(), r_assign->group) != sg_in_slot.end()) {
                    cout << "ERROR: " << (*r_assign->group).name << " found assigned in slot "
                         << *r_slot << " more than once" << endl;
                    is_sol_valid = false;
                }
                sg_in_slot.push_back(r_assign->group);
            }
        }
    }
    return is_sol_valid;
};

SolutionEvaluation::SolutionEvaluation(unique_ptr<Data>& p_data, int num_assign,
                                       vector<int>& r_num_assign_by_slot, vector<int>& r_num_assign_by_day,
                                       vector<int>& r_num_assign_by_pro, vector<int>& r_num_assign_by_group,
                                       float std_assign_by_slot, float std_assign_by_day,
                                       float std_assign_by_pro, float std_assign_by_group)
        : p_data(p_data), num_assign(num_assign), num_assign_by_day(r_num_assign_by_day),
          num_assign_by_pro(r_num_assign_by_pro), num_assign_by_group(r_num_assign_by_group),
          std_assign_by_slot(std_assign_by_slot), std_assign_by_day(std_assign_by_day),
          std_assign_by_pro(std_assign_by_pro), std_assign_by_group(std_assign_by_group) {}

ostream& SolutionEvaluation::print(ostream& os) const {
    os << "SolutionEvaluation(Number of assignations : " << num_assign << endl;
    os << "Standard deviation of the number of assignations by slot " << std_assign_by_slot << endl;
    os << "Standard deviation of the assignations by day " << std_assign_by_day << endl;
    os << "Standard deviation of the assignations by professional " << std_assign_by_pro << endl;
    os << "Standard deviation of the assignations by group " << std_assign_by_group << endl;
    os << "Number of assignations by pro :" << endl;
    for (auto const& rPro : p_data->professionals) {
        os << "    " << rPro->name << " : " << num_assign_by_pro[rPro->idx] << " / "
           << rPro->slots.size() << endl;
    }
    return os;
};

SolutionEvaluation* evaluate(unique_ptr<Data>& p_data, unique_ptr<Solution>& p_sol) {
    int num_assign = p_sol->assignations.size();
    vector<int> num_assign_by_slot(p_data->dimensions.num_slots, 0);
    vector<int> num_assign_by_day(p_data->config.nb_days, 0);
    vector<int> num_assign_by_pro(p_data->dimensions.num_pros, 0);
    vector<int> num_assign_by_group(p_data->dimensions.num_groups, 0);
    for (auto const& r_assign : p_sol->assignations) {
        num_assign_by_slot[r_assign->slot->idx]++;
        num_assign_by_day[r_assign->slot->day]++;
        num_assign_by_pro[r_assign->pro->idx]++;
        num_assign_by_group[r_assign->group->idx]++;
    }
    float std_assign_by_slot = computeSDVec(num_assign_by_slot);
    float std_assign_by_day = computeSDVec(num_assign_by_day);
    float std_assign_by_pro = computeSDVec(num_assign_by_pro);
    float std_assign_by_group = computeSDVec(num_assign_by_group);
    SolutionEvaluation* p_sol_eval = new SolutionEvaluation(p_data, num_assign, num_assign_by_slot,
                                                            num_assign_by_day, num_assign_by_pro,
                                                            num_assign_by_group, std_assign_by_slot,
                                                            std_assign_by_day, std_assign_by_pro,
                                                            std_assign_by_group);

    return p_sol_eval;
};