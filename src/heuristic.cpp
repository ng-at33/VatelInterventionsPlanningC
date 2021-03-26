/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.cpp
 */

#include "data.h"
#include "heuristic.h"

using namespace std;

int HeurNode::NODE_COUNTER(0);

HeurNode::HeurNode(Data& data) : id(NODE_COUNTER++), cost(0),
    slots(vector<set<pair<Professional*, StudentGroup*>>>(data.dimensions.numSlots)) {
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

HeurNode* firstFit(Data& data, HeurNode* node) {
};

HeurNode* firstFit(Data& data) {
    auto* firstNode = new HeurNode(data);

    vector<vector<bool>> isIntervByGrSl(data.dimensions.numGroups,
        vector<bool> (data.dimensions.numSlots, false));
    vector<vector<bool>> isIntervByPrSl(data.dimensions.numPros,
        vector<bool> (data.dimensions.numSlots, false));
    vector<vector<int>> nbIntervByPrDa(data.dimensions.numPros,
        vector<int> (data.config.nbDays, 0));
    vector<int> nbIntervByPr(data.dimensions.numPros, 0);
    vector<int> nbIntervByGr(data.dimensions.numGroups, 0);
    vector<int> nbIntervBySl(data.dimensions.numSlots, 0);

    // Filling empty node
    // Sorting slots by least pros available
    auto slots = data.slots;
    std::sort(slots.begin(), slots.end(),
        [] (TimeSlot* ts1, TimeSlot* ts2) {
            return ts1->pros.size() < ts2->pros.size();
        });

    // Iterating over sorted slots
    for (auto& slot : slots) {
        // Sorting pros by least compatible
        auto pros = data.professionals;
        std::sort(pros.begin(), pros.end(),
            [] (Professional* pro1, Professional* pro2) {
                return pro1->slots.size() < pro2->slots.size();
            });
        cout << *slot << endl;
        for (auto itPros = pros.begin();
                itPros != pros.end() && itPros != pros.begin() + 3; itPros++) {
            // TODO : search in groups compatible with pro (languages)
            int chosenGroupIdx = min_element(nbIntervByGr.begin(),
                nbIntervByGr.end()) - nbIntervByGr.begin();
            cout << chosenGroupIdx << endl;
            cout << data.groups.size() << endl;
            cout << *data.groups[chosenGroupIdx] << endl;
            auto group = data.groups[chosenGroupIdx];
            // Creating chosen <Professional*, StudenGroup*> pair
            pair<Professional*, StudentGroup*> pairPrGr = make_pair(*itPros,
                group);
            cout << " slot idx " << slot->idx << endl;
            firstNode->slots[slot->idx].insert(pairPrGr);
            isIntervByGrSl[chosenGroupIdx][slot->idx] = true;
            isIntervByPrSl[(*itPros)->idx][slot->idx] = true;
            nbIntervByPrDa[(*itPros)->idx][slot->day]++;
            nbIntervByPr[(*itPros)->idx]++;
            nbIntervByGr[chosenGroupIdx]++;
            nbIntervBySl[slot->idx]++;
        }
    }
    cout << "firstNode" << *firstNode << endl;
    return firstNode;
};

