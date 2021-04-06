/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.cpp
 */

#include <set>
#include <utility>

#include "data.h"
#include "heuristic.h"
#include "utils.h"

using namespace std;

int HeurNode::NODE_COUNTER(0);

// Create an fully filled node
HeurNode::HeurNode(Data* data, float cost, vector<vector<bool> >& isIntervByGrSl,
        vector<vector<bool> >& isIntervByPrSl, vector<vector<int> >& nbIntervByPrDa,
        vector<int>& nbIntervByPr, vector<int>& nbIntervByGr, vector<int>& nbIntervBySl,
        vector<int>& nbIntervByDa, vector<std::set<std::pair<Professional*, StudentGroup*> > >& slots,
        set<std::pair<Professional*, StudentGroup*> >& assignations)
        : id(NODE_COUNTER++), cost(cost), isIntervByGrSl(isIntervByGrSl),
        isIntervByPrSl(isIntervByPrSl), nbIntervByPrDa(nbIntervByPrDa), nbIntervByPr(nbIntervByPr),
        nbIntervByGr(nbIntervByGr), nbIntervBySl(nbIntervBySl), nbIntervByDa(nbIntervByDa),
        slots(slots), assignations(assignations) {
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

float evaluate(Data* data, vector<vector<bool> >& isIntervByGrSl, vector<int>& nbIntervByPr,
        vector<int>& nbIntervByGr, vector<int>& nbIntervBySl, vector<int>& nbIntervByDa) {
    float fitness = 0.0;
    // fitness += computeSDVec(nbIntervByPr);
    // fitness += computeSDVec(nbIntervByGr);
    // fitness += computeSDVec(nbIntervBySl);
    // fitness += computeSDVec(nbIntervByDa);
    float avgStdGroupSlot = 0.0;
    for (auto& groupVec : isIntervByGrSl) {
        vector<int> allIndexesTrue;
        for (int slotIdx = 0; slotIdx < data->dimensions.numSlots; slotIdx++) {
            if (groupVec[slotIdx]) allIndexesTrue.push_back(slotIdx);
        }
        auto stdd = computeSDVec(allIndexesTrue);
        for (int slotIdx = 0; slotIdx < data->dimensions.numSlots; slotIdx++) {
            if (groupVec[slotIdx]) {
                allIndexesTrue.push_back(slotIdx);
            }
        }

        avgStdGroupSlot += stdd;
    }
    fitness += avgStdGroupSlot / data->dimensions.numGroups;
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
    vector<set<pair<Professional*, StudentGroup*> > > slots(data->dimensions.numSlots);
    set<pair<Professional*, StudentGroup*> > assignations;
    // Filling empty node
    // Sorting slots by least pros available
    auto slotsTmp = data->slots;
    sort(slotsTmp.begin(), slotsTmp.end(),
        [] (TimeSlot* ts1, TimeSlot* ts2) {
            return ts1->pros.size() < ts2->pros.size();
        });

    // Iterating over sorted slots
    for (auto& slot : slotsTmp) {
        if (nbIntervBySl[slot->idx] >= 3) continue;
        // Sorting pros by least compatible
        auto pros = data->professionals;
        sort(pros.begin(), pros.end(),
            [] (Professional* pro1, Professional* pro2) {
                return pro1->slots.size() < pro2->slots.size();
            });
        for (auto itPros = pros.begin(); itPros != pros.end(); itPros++) {
            // Check if slot has not already reached its max number of interventions
            if (nbIntervBySl[slot->idx] >= 3) break;
            // Check if professional has not already reached his max number of interventions
            if (nbIntervByPr[(*itPros)->idx] >= 3) continue;
            // Check if professional is available on this slot
            if (find((*itPros)->slots.begin(), (*itPros)->slots.end(), slot) ==
                (*itPros)->slots.end()) continue;
            StudentGroup* group;
            // TODO : search in groups compatible with pro (languages)
            // Check if group and professionals are not already assigned to slot
            for (auto itGrp = data->groups.begin();
                    itGrp != data->groups.end(); itGrp++) {
                if (!isIntervByGrSl[(*itGrp)->idx][slot->idx] &&
                        !isIntervByPrSl[(*itPros)->idx][slot->idx]) {
                    group = *itGrp;
                    break;
                }
            }
            if (group == NULL) continue;
            // Check if <pro, group> is not already assigned
            if (find(assignations.begin(), assignations.end(), make_pair(*itPros, group)) !=
                assignations.end()) continue;
            // Creating chosen <Professional*, StudenGroup*> pair
            pair<Professional*, StudentGroup*> pairPrGr = make_pair(*itPros, group);
            slots[slot->idx].insert(pairPrGr);
            assignations.insert(pairPrGr);
            isIntervByGrSl[group->idx][slot->idx] = true;
            isIntervByPrSl[(*itPros)->idx][slot->idx] = true;
            nbIntervByPrDa[(*itPros)->idx][slot->day]++;
            nbIntervByPr[(*itPros)->idx]++;
            nbIntervByGr[group->idx]++;
            nbIntervBySl[slot->idx]++;
            nbIntervByDa[slot->day]++;
        }
    }

    float cost = evaluate(data, isIntervByGrSl, nbIntervByPr, nbIntervByGr, nbIntervBySl,
        nbIntervByDa);
    auto* firstNode = new HeurNode(data, cost, isIntervByGrSl, isIntervByPrSl, nbIntervByPrDa,
        nbIntervByPr, nbIntervByGr, nbIntervBySl, nbIntervByDa, slots, assignations);

    return firstNode;
};

vector<HeurNode*> HeurNode::generateSwaps(Data* data) {
    std::vector<HeurNode*> swappedNodes;
    auto slotVec = this->slots.begin();
    for (int slotIdx = 0; slotVec != this->slots.end(); slotVec++, slotIdx++) {
        TimeSlot* slot = data->slots[slotIdx];
        auto pair = slotVec->begin();
        for (int pairI = 0; pair != slotVec->end(); pair++, pairI++) {
            auto oslotVec = this->slots.begin();
            for (int oslotIdx = 0; oslotVec != this->slots.end(); oslotVec++, oslotIdx++) {
                if (slotIdx != oslotIdx) {
                    TimeSlot* oslot = data->slots[oslotIdx];
                    auto opair = oslotVec->begin();
                    for (int opairI = 0; opair != oslotVec->end(); opair++, opairI++) {
                        if (*opair == *pair) continue;
                        // Swap assignations if possible by creating a new node
                        // Checking if professionals can be swapped
                        if (pair->first != opair->first) {
                            if (!isProSwappable(data, oslotIdx, pair->first)) continue;
                            if (!isProSwappable(data, slotIdx, opair->first)) continue;
                        }
                        // Checking if groups can be swapped
                        if (pair->second != opair->second) {
                            if (!isGroupSwappable(oslotIdx, pair->second)) continue;
                            if (!isGroupSwappable(slotIdx, opair->second)) continue;
                        }
                        // Create new node with swapped pros and groups
                        vector<vector<bool> > isIntervByGrSl(this->isIntervByGrSl);
                        vector<vector<bool> > isIntervByPrSl(this->isIntervByPrSl);
                        vector<vector<int> > nbIntervByPrDa(this->nbIntervByPrDa);
                        vector<int> nbIntervByPr(this->nbIntervByPr);
                        vector<int> nbIntervByGr(this->nbIntervByGr);
                        vector<int> nbIntervBySl(this->nbIntervBySl);
                        vector<int> nbIntervByDa(this->nbIntervByDa);
                        vector<set<std::pair<Professional*, StudentGroup*> > > slots(this->slots);
                        set<std::pair<Professional*, StudentGroup*> > assignations(this->assignations);
                        isIntervByGrSl[pair->second->idx][slotIdx] = false;
                        isIntervByGrSl[opair->second->idx][oslotIdx] = false;
                        isIntervByGrSl[pair->second->idx][oslotIdx] = true;
                        isIntervByGrSl[opair->second->idx][slotIdx] = true;
                        isIntervByPrSl[pair->first->idx][slotIdx] = false;
                        isIntervByPrSl[opair->first->idx][oslotIdx] = false;
                        isIntervByPrSl[pair->first->idx][oslotIdx] = true;
                        isIntervByPrSl[opair->first->idx][slotIdx] = true;
                        nbIntervByPrDa[pair->first->idx][slot->day]--;
                        nbIntervByPrDa[opair->first->idx][oslot->day]--;
                        nbIntervByPrDa[pair->first->idx][oslot->day]++;
                        nbIntervByPrDa[opair->first->idx][slot->day]++;
                        auto assign1 = find_if(slots[slotIdx].begin(), slots[slotIdx].end(),
                            [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
                                return tmpPair.first == pair->first;
                            });
                        auto assign2 = find_if(slots[oslotIdx].begin(), slots[oslotIdx].end(),
                            [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
                                return tmpPair.first == opair->first;
                            });
                        slots[slotIdx].insert(*assign2);
                        slots[oslotIdx].insert(*assign1);
                        slots[slotIdx].erase(assign1);
                        slots[oslotIdx].erase(assign2);
                        float cost = evaluate(data, isIntervByGrSl, nbIntervByPr, nbIntervByGr,
                            nbIntervBySl, nbIntervByDa);
                        HeurNode* swappedNode = new HeurNode(data, cost, isIntervByGrSl, isIntervByPrSl,
                            nbIntervByPrDa, nbIntervByPr, nbIntervByGr, nbIntervBySl, nbIntervByDa,
                            slots, assignations);
                        swappedNodes.push_back(swappedNode);
                    }
                }
            }
        }
    }
    return swappedNodes;
}

bool HeurNode::isProSwappable(Data* data, int slotIdx, Professional* pro) {
    TimeSlot* slot = data->slots[slotIdx];
    int day = slot->day;
    int proIdx = pro->idx;
    if (nbIntervByPr[proIdx] >= 3) return false;
    if (nbIntervByPrDa[day][proIdx] >= 3) return false;
    if (isIntervByPrSl[pro->idx][slotIdx]) return false;
    return true;
}

bool HeurNode::isGroupSwappable(int slotIdx, StudentGroup* group) {
    if (isIntervByGrSl[group->idx][slotIdx]) return false;
    return true;
}

std::vector<HeurNode*> iterate(Data* data, std::vector<HeurNode*> nodes) {
    std::vector<HeurNode*> newNodes;
    // Generating swaps of all nodes
    for (auto& node : nodes) {
        vector<HeurNode*> swaps = node->generateSwaps(data);
        sort(swaps.begin(), swaps.end(),
            [] (HeurNode* node1, HeurNode* node2) {
                return node1->cost < node2->cost;
            });
        newNodes.insert(newNodes.end(), swaps.begin(), swaps.end());
    }
    return newNodes;
}