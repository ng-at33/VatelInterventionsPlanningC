/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.cpp
 */

#include "data.h"
#include "heuristic.h"

using namespace std;

int HeurNode::NODE_COUNTER(0);

HeurNode::HeurNode(Data* data) : id(NODE_COUNTER++), cost(0),
    slots(vector<set<pair<Professional*, StudentGroup*>>>(data->dimensions.numSlots)) {
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

HeurNode* firstFit(Data* data, HeurNode* node) {
};

HeurNode* firstFit(Data* data) {
    auto* firstNode = new HeurNode(data);

    vector<vector<bool>> isIntervByGrSl(data->dimensions.numGroups,
        vector<bool> (data->dimensions.numSlots, false));
    vector<vector<bool>> isIntervByPrSl(data->dimensions.numPros,
        vector<bool> (data->dimensions.numSlots, false));
    vector<vector<int>> nbIntervByPrDa(data->dimensions.numPros,
        vector<int> (data->config.nbDays, 0));
    vector<int> nbIntervByPr(data->dimensions.numPros, 0);
    vector<int> nbIntervByGr(data->dimensions.numGroups, 0);
    vector<int> nbIntervBySl(data->dimensions.numSlots, 0);

    // Filling empty node
    // Sorting slots by least pros available
    auto slots = data->slots;
    std::sort(slots.begin(), slots.end(),
        [] (TimeSlot* ts1, TimeSlot* ts2) {
            return ts1->pros.size() < ts2->pros.size();
        });

    // Iterating over sorted slots
    for (auto& slot : slots) {
        if (nbIntervBySl[slot->idx] >= 3) continue;
        // Sorting pros by least compatible
        auto pros = data->professionals;
        std::sort(pros.begin(), pros.end(),
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
            firstNode->slots[slot->idx].insert(pairPrGr);
            isIntervByGrSl[group->idx][slot->idx] = true;
            isIntervByPrSl[(*itPros)->idx][slot->idx] = true;
            nbIntervByPrDa[(*itPros)->idx][slot->day]++;
            nbIntervByPr[(*itPros)->idx]++;
            nbIntervByGr[group->idx]++;
            nbIntervBySl[slot->idx]++;
        }
    }
    cout << "firstNode" << *firstNode << endl;
    return firstNode;
};

