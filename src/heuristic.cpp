/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.cpp
 */

#include "data.h"
#include "heuristic.h"
#include "utils.h"

using namespace std;

int HeurNode::NODE_COUNTER(0);

HeurNode::HeurNode(Data* data, float cost, vector<vector<bool> >& isIntervByGrSl,  // Create an fully filled node
        vector<vector<bool> >& isIntervByPrSl, vector<vector<int> >& nbIntervByPrDa,
        vector<int>& nbIntervByPr, vector<int>& nbIntervByGr, vector<int>& nbIntervBySl,
        vector<int>& nbIntervByDa, vector<std::set<std::pair<Professional*, StudentGroup*>>>& slots)
        : id(NODE_COUNTER++), cost(cost), isIntervByGrSl(isIntervByGrSl),
        isIntervByPrSl(isIntervByPrSl), nbIntervByPrDa(nbIntervByPrDa), nbIntervByPr(nbIntervByPr),
        nbIntervByGr(nbIntervByGr), nbIntervBySl(nbIntervBySl), nbIntervByDa(nbIntervByDa),
        slots(slots) {
    NODE_COUNTER++;
};

ostream& HeurNode::print(ostream& os) const {
    os << "Node(id : " << id << ", cost : " << cost << ", slots : " << endl;
    auto cntSlot = 0;
    for (auto& slot : slots) {
        os << " " << cntSlot << " : ";
        for (auto& slotCnt : slot) {
            os << "(" << (*slotCnt.first).name << ", " << (*slotCnt.second).name << ") ";
        }
        os << endl;
        cntSlot++;
    }
    os << ")";
    return os;
};

float evaluate(vector<int>& nbIntervByPr, vector<int>& nbIntervByGr, vector<int>& nbIntervBySl,
        vector<int>& nbIntervByDa) {
    float fitness = 0.0;

    fitness += computeSDVec(nbIntervByPr);
    fitness += computeSDVec(nbIntervByGr);
    fitness += computeSDVec(nbIntervBySl);
    fitness += computeSDVec(nbIntervByDa);

    return fitness;
};

HeurNode* firstFit(Data* data) {
    vector<vector<bool> > isIntervByGrSl(data->dimensions.numGroups, vector<bool>(data->dimensions.numSlots, false));
    vector<vector<bool> > isIntervByPrSl(data->dimensions.numPros, vector<bool>(data->dimensions.numSlots, false));
    vector<vector<int> > nbIntervByPrDa(data->dimensions.numPros, vector<int>(data->config.nbDays, 0));
    vector<int> nbIntervByPr(data->dimensions.numPros, 0);
    vector<int> nbIntervByGr(data->dimensions.numGroups, 0);
    vector<int> nbIntervBySl(data->dimensions.numSlots, 0);
    vector<int> nbIntervByDa(data->config.nbDays, 0);
    vector<set<pair<Professional*, StudentGroup*>>> slots(data->dimensions.numSlots);
    // Filling empty node
    // Sorting slots by least pros available
    auto slots_tmp = data->slots;
    sort(slots_tmp.begin(), slots_tmp.end(),
        [] (TimeSlot* ts1, TimeSlot* ts2) {
            return ts1->pros.size() < ts2->pros.size();
        });

    // Iterating over sorted slots
    for (auto& slot : slots_tmp) {
        if (nbIntervBySl[slot->idx] >= 3) continue;
        // Sorting pros by least compatible
        auto pros = data->professionals;
        sort(pros.begin(), pros.end(),
            [] (Professional* pro1, Professional* pro2) {
                return pro1->slots.size() < pro2->slots.size();
            });
        for (auto itPros = pros.begin(); itPros != pros.end(); itPros++) {
            if (nbIntervBySl[slot->idx] >= 3) break;
            if (nbIntervByPr[(*itPros)->idx] >= 3) continue;
            if (find((*itPros)->slots.begin(), (*itPros)->slots.end(), slot) ==
                (*itPros)->slots.end()) continue;
            StudentGroup* group;
            // auto chosenGroupVal = -1;
            // TODO : search in groups compatible with pro (languages)
            for (auto itGrp = data->groups.begin();
                    itGrp != data->groups.end(); itGrp++) {
                if (!isIntervByGrSl[(*itGrp)->idx][slot->idx] &&
                        !isIntervByPrSl[(*itPros)->idx][slot->idx]) {
                    group = *itGrp;
                    break;
                }
            }
            if (group == NULL) {
                continue;
            }
            // Creating chosen <Professional*, StudenGroup*> pair
            pair<Professional*, StudentGroup*> pairPrGr = make_pair(*itPros, group);
            slots[slot->idx].insert(pairPrGr);
            isIntervByGrSl[group->idx][slot->idx] = true;
            isIntervByPrSl[(*itPros)->idx][slot->idx] = true;
            nbIntervByPrDa[(*itPros)->idx][slot->day]++;
            nbIntervByPr[(*itPros)->idx]++;
            nbIntervByGr[group->idx]++;
            nbIntervBySl[slot->idx]++;
            nbIntervByDa[slot->day]++;
        }
    }

    float cost = evaluate(nbIntervByPr, nbIntervByGr, nbIntervBySl, nbIntervByDa);
    auto* firstNode = new HeurNode(data, cost, isIntervByGrSl, isIntervByPrSl, nbIntervByPrDa,
        nbIntervByPr, nbIntervByGr, nbIntervBySl, nbIntervByDa, slots);
    cout << "firstNode" << *firstNode << endl;
    return firstNode;
};

