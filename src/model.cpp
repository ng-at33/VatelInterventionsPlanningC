/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * model.cpp
 */

#include <map>
#include <string>
#include <tuple>

#include "model.h"
#include "data.h"

using namespace std;
using namespace operations_research;

VatelModel* buildModel(Data& data, MPSolver& solver) {
    LOG(INFO) << "Creating model ! " << std::endl;
    // Defining constants
    const double infinity = solver.infinity();
    // Objective
    MPObjective* const objective = solver.MutableObjective();
    objective->SetMaximization();
    // Variables
    map<tuple<int, int>, MPVariable*> xVarMap {};
    for (auto& pro : data.professionals) {
        for (auto& slot : pro->slots) {
            cout << *slot << endl;
            MPVariable* const new_x = solver.MakeBoolVar("x_p" +
                std::to_string(pro->idx) + "t" + std::to_string(slot->idx));
            xVarMap[make_tuple(pro->idx, slot->idx)] = new_x;
            objective->SetCoefficient(new_x, 1);
        }
    }
    // Constraints
    // Upper bound on each slot
    for (auto& slot : data.slots) {
        MPConstraint* const new_c = 
            solver.MakeRowConstraint(0, 2, "ub_slot_t" + to_string(slot->idx));
        for (auto& pro : data.professionals) {
            auto xVar = xVarMap.find(make_tuple(pro->idx, slot->idx));
            new_c->SetCoefficient(xVar->second, 1);
        }
    }
    // Upper bound on each pro
    for (auto& pro : data.professionals) {
        MPConstraint* const new_c = 
            solver.MakeRowConstraint(0, data.config.max_inter,
                "ub_pro_p" + to_string(pro->idx));
        for (auto& slot : pro->slots) {
            auto xVar = xVarMap.find(make_tuple(pro->idx, slot->idx));
            new_c->SetCoefficient(xVar->second, 1);
        }
    }
    VatelModel* vatelModel = new VatelModel();
    vatelModel->xVarMap = xVarMap;

    return vatelModel;
};