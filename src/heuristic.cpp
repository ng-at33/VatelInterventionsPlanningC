/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.cpp
 */

#include "data.h"
#include "heuristic.h"

using namespace std;

int HeurNode::NODE_COUNTER(0);

HeurNode::HeurNode(Data* data) : id(NODE_COUNTER++), cost(0),
    isIntervByGrSl(vector<vector<bool> > (data->dimensions.numGroups,
        vector<bool>(data->dimensions.numSlots, false))),
    isIntervByPrSl(vector<vector<bool> > (data->dimensions.numPros,
        vector<bool>(data->dimensions.numSlots, false))),
    nbIntervByPrDa(vector<vector<int> > (data->dimensions.numPros,
        vector<int>(data->config.nbDays, 0))),
    nbIntervByPr(vector<int>(data->dimensions.numPros, 0)),
    nbIntervByGr(vector<int>(data->dimensions.numGroups, 0)),
    nbIntervBySl(vector<int>(data->dimensions.numSlots, 0)),
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
    // Filling empty node
    // Sorting slots by least pros available
    auto slots = data->slots;
    std::sort(slots.begin(), slots.end(),
        [] (TimeSlot* ts1, TimeSlot* ts2) {
            return ts1->pros.size() < ts2->pros.size();
        });

    // Iterating over sorted slots
    for (auto& slot : slots) {
        if (firstNode->nbIntervBySl[slot->idx] >= 3) continue;
        // Sorting pros by least compatible
        auto pros = data->professionals;
        std::sort(pros.begin(), pros.end(),
            [] (Professional* pro1, Professional* pro2) {
                return pro1->slots.size() < pro2->slots.size();
            });
        for (auto itPros = pros.begin(); itPros != pros.end(); itPros++) {
            if (firstNode->nbIntervBySl[slot->idx] >= 3) break;
            if (firstNode->nbIntervByPr[(*itPros)->idx] >= 3) continue;
            if (find((*itPros)->slots.begin(), (*itPros)->slots.end(), slot) ==
                (*itPros)->slots.end()) continue;
            StudentGroup* group;
            // auto chosenGroupVal = -1;
            // TODO : search in groups compatible with pro (languages)
            for (auto itGrp = data->groups.begin();
                    itGrp != data->groups.end(); itGrp++) {
                if (!firstNode->isIntervByGrSl[(*itGrp)->idx][slot->idx] &&
                        !firstNode->isIntervByPrSl[(*itPros)->idx][slot->idx]) {
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
            firstNode->isIntervByGrSl[group->idx][slot->idx] = true;
            firstNode->isIntervByPrSl[(*itPros)->idx][slot->idx] = true;
            firstNode->nbIntervByPrDa[(*itPros)->idx][slot->day]++;
            firstNode->nbIntervByPr[(*itPros)->idx]++;
            firstNode->nbIntervByGr[group->idx]++;
            firstNode->nbIntervBySl[slot->idx]++;
        }
    }
    cout << "firstNode" << *firstNode << endl;
    return firstNode;
};

