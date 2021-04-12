/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.cpp
 */

#include <algorithm>
#include <set>
#include <utility>

#include "data.h"
#include "heuristic.h"
#include "utils.h"

using namespace std;

int HeurNode::NODE_COUNTER(0);

HeurNode::HeurNode(Data* data)
        : id(NODE_COUNTER++), cost(0.0), isIntervByGrSl(data->dimensions.numGroups, vector<bool>(data->dimensions.numSlots, false)),
        isIntervByPrSl(data->dimensions.numPros, vector<bool>(data->dimensions.numSlots, false)),
        nbIntervByPrDa(data->dimensions.numPros, vector<int>(data->config .nbDays, 0)),
        nbIntervByPr(data->dimensions.numPros, 0),
        nbIntervByGr(data->dimensions.numGroups, 0),
        nbIntervBySl(data->dimensions.numSlots, 0),
        nbIntervByDa(data->config.nbDays, 0),
        slots(data->dimensions.numSlots, set<pair<Professional*, StudentGroup*> >()),
        assignations() {};

HeurNode::HeurNode(HeurNode* node)
        : id(NODE_COUNTER++), cost(node->cost), isIntervByGrSl(node->isIntervByGrSl),
        isIntervByPrSl(node->isIntervByPrSl), nbIntervByPrDa(node->nbIntervByPrDa),
        nbIntervByPr(node->nbIntervByPr), nbIntervByGr(node->nbIntervByGr),
        nbIntervBySl(node->nbIntervBySl), nbIntervByDa(node->nbIntervByDa),
        slots(node->slots), assignations(node->assignations) {};

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

void HeurNode::evaluate() {
    cost = 0.0;
    cost += computeSDVec(nbIntervByPr);
    cost += computeSDVec(nbIntervByGr);
    cost += computeSDVec(nbIntervBySl);
    cost += computeSDVec(nbIntervByDa);
};

HeurNode* firstFit(Data* data) {
    auto* firstNode = new HeurNode(data);
    // Filling empty node
    // Sorting slots by least pros available
    auto slotsTmp = data->slots;
    sort(slotsTmp.begin(), slotsTmp.end(),
        [] (TimeSlot* ts1, TimeSlot* ts2) {
            return ts1->pros.size() < ts2->pros.size();
        });

    // Iterating over sorted slots
    for (auto& slot : slotsTmp) {
        // Sorting pros by least compatible
        auto pros = data->professionals;
        sort(pros.begin(), pros.end(),
            [] (Professional* pro1, Professional* pro2) {
                return pro1->slots.size() < pro2->slots.size();
            });
        for (auto itPros = pros.begin(); itPros != pros.end(); itPros++) {
            for (auto itGroup = data->groups.begin(); itGroup != data->groups.end(); itGroup++) {
                if (!firstNode->isProAssignable(data, slot, *itPros)) continue;
                if (!firstNode->isGroupAssignable(*itGroup, slot)) continue;
                if (!firstNode->isProGroupAssignable(*itPros, *itGroup)) continue;
                if (!firstNode->isSlotAssignable(slot)) continue;
                // Creating chosen <Professional*, StudenGroup*> pair
                pair<Professional*, StudentGroup*> pairPrGr = make_pair(*itPros, *itGroup);
                firstNode->slots[slot->idx].insert(pairPrGr);
                firstNode->assignations.insert(pairPrGr);
                firstNode->isIntervByGrSl[(*itGroup)->idx][slot->idx] = true;
                firstNode->isIntervByPrSl[(*itPros)->idx][slot->idx] = true;
                firstNode->nbIntervByPrDa[(*itPros)->idx][slot->day]++;
                firstNode->nbIntervByPr[(*itPros)->idx]++;
                firstNode->nbIntervByGr[(*itGroup)->idx]++;
                firstNode->nbIntervBySl[slot->idx]++;
                firstNode->nbIntervByDa[slot->day]++;
            }
        }
    }
    firstNode->evaluate();

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
                            if (!isProAssignable(data, oslot, pair->first)) continue;
                            if (!isProAssignable(data, slot, opair->first)) continue;
                        }
                        // Checking if groups can be swapped
                        if (pair->second != opair->second) {
                            if (!isGroupAssignable(pair->second, oslot)) continue;
                            if (!isGroupAssignable(opair->second, slot)) continue;
                        }
                        // Create new node with swapped pros and groups
                        HeurNode* swappedNode = new HeurNode(this);
                        swappedNode->isIntervByGrSl[pair->second->idx][slotIdx] = false;
                        swappedNode->isIntervByGrSl[opair->second->idx][oslotIdx] = false;
                        swappedNode->isIntervByGrSl[pair->second->idx][oslotIdx] = true;
                        swappedNode->isIntervByGrSl[opair->second->idx][slotIdx] = true;
                        swappedNode->isIntervByPrSl[pair->first->idx][slotIdx] = false;
                        swappedNode->isIntervByPrSl[opair->first->idx][oslotIdx] = false;
                        swappedNode->isIntervByPrSl[pair->first->idx][oslotIdx] = true;
                        swappedNode->isIntervByPrSl[opair->first->idx][slotIdx] = true;
                        swappedNode->nbIntervByPrDa[pair->first->idx][slot->day]--;
                        swappedNode->nbIntervByPrDa[opair->first->idx][oslot->day]--;
                        swappedNode->nbIntervByPrDa[pair->first->idx][oslot->day]++;
                        swappedNode->nbIntervByPrDa[opair->first->idx][slot->day]++;
                        auto assign1 = swappedNode->findAsInSlot(slotIdx, pair->first, pair->second);
                        auto assign2 = swappedNode->findAsInSlot(slotIdx, opair->first, opair->second);
                        swappedNode->slots[slotIdx].insert(*assign2);
                        swappedNode->slots[oslotIdx].insert(*assign1);
                        swappedNode->slots[slotIdx].erase(assign1);
                        swappedNode->slots[oslotIdx].erase(assign2);
                        swappedNode->evaluate();
                        swappedNodes.push_back(swappedNode);
                    }
                }
            }
        }
    }
    return swappedNodes;
}

vector<HeurNode*> HeurNode::generateMutationsAssignations(Data* data) {
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
                    if (!pair->first->isProAvailOnSlot(oslot)) continue;
                    if (isNbIntervByPrDaReached(nbIntervByPrDa, pair->first, oslot)) continue;
                    if (isIntervPrSlAlready(isIntervByPrSl, pair->first, oslot)) continue;
                    if (isIntervGrSlAlready(isIntervByGrSl, pair->second, oslot)) continue;
                    if (isNbIntervSlReached(nbIntervBySl, oslot)) continue;
                    // Create new node with assignation moved from slot to oslot
                    HeurNode* swappedNode = new HeurNode(this);
                    swappedNode->isIntervByGrSl[pair->second->idx][slotIdx] = false;
                    swappedNode->isIntervByGrSl[pair->second->idx][oslotIdx] = true;
                    swappedNode->isIntervByPrSl[pair->first->idx][slotIdx] = false;
                    swappedNode->isIntervByPrSl[pair->first->idx][oslotIdx] = true;
                    swappedNode->nbIntervByPrDa[pair->first->idx][slot->day]--;
                    swappedNode->nbIntervByPrDa[pair->first->idx][oslot->day]++;
                    swappedNode->nbIntervBySl[slotIdx]--;
                    swappedNode->nbIntervBySl[oslotIdx]++;
                    swappedNode->nbIntervByDa[slot->day]--;
                    swappedNode->nbIntervByDa[oslot->day]++;
                    auto assign = swappedNode->findAsInSlot(slotIdx, pair->first, pair->second);
                    swappedNode->slots[slotIdx].erase(assign);
                    swappedNode->slots[oslotIdx].insert(*assign);
                    swappedNode->evaluate();
                    swappedNodes.push_back(swappedNode);
                }
            }
        }
    }
    return swappedNodes;
}

vector<HeurNode*> HeurNode::generateMutationsGroups(Data* data) {
    std::vector<HeurNode*> swappedNodes;
    auto slotVec = this->slots.begin();
    for (int slotIdx = 0; slotVec != this->slots.end(); slotVec++, slotIdx++) {
        auto slot = data->slots[slotIdx];
        auto pair = slotVec->begin();
        for (int pairI = 0; pair != slotVec->end(); pair++, pairI++) {
            for (auto& group : data->groups) {
                if (pair->second != group) {
                    if (isIntervGrSlAlready(isIntervByGrSl, group, slot)) continue;
                    if (!isProGroupAssignable(pair->first, group)) continue;
                    // Create new node with group changed
                    HeurNode* swappedNode = new HeurNode(this);
                    swappedNode->isIntervByGrSl[pair->second->idx][slotIdx] = false;
                    swappedNode->isIntervByGrSl[group->idx][slotIdx] = true;
                    swappedNode->nbIntervByGr[pair->second->idx]--;
                    swappedNode->nbIntervByGr[group->idx]++;
                    auto assignSlot = swappedNode->findAsInSlot(slotIdx, pair->first, pair->second);
                    swappedNode->slots[slotIdx].erase(assignSlot);
                    std::pair<Professional*, StudentGroup*> pairPrGr = make_pair(pair->first, group);
                    swappedNode->slots[slotIdx].insert(pairPrGr);
                    auto assignAs = swappedNode->findAs(pair->first, pair->second);
                    swappedNode->assignations.erase(assignAs);
                    swappedNode->assignations.insert(pairPrGr);
                    swappedNode->evaluate();
                    swappedNodes.push_back(swappedNode);
                }
            }
        }
    }
    return swappedNodes;
}

bool HeurNode::isProAssignable(Data* data, TimeSlot* slot, Professional* pro) {
    int day = slot->day;
    if (!pro->isProAvailOnSlot(slot)) return false;
    if (isNbIntervByProReached(nbIntervByPr, pro)) return false;
    if (isNbIntervByPrDaReached(nbIntervByPrDa, pro, slot)) return false;
    if (isIntervPrSlAlready(isIntervByPrSl, pro, slot)) return false;
    return true;
}

bool HeurNode::isSlotAssignable(TimeSlot* slot) {
    if (isNbIntervSlReached(nbIntervBySl, slot)) return false;
    return true;
}

bool HeurNode::isGroupAssignable(StudentGroup* group, TimeSlot* slot) {
    if (isIntervGrSlAlready(isIntervByGrSl, group, slot)) return false;
    return true;
}

bool HeurNode::isProGroupAssignable(Professional* pro, StudentGroup* group) {
    // Check if <pro, group> is not already assigned
    auto tmpPair = make_pair(pro, group);
    if (find(assignations.begin(), assignations.end(), tmpPair) != assignations.end()) {
        return false;
    }
    if (!pro->isGroupCompatible(group)) return false;
    return true;
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsInSlot(int slotIdx,
        Professional* pro, StudentGroup* group) {
    return find_if(slots[slotIdx].begin(), slots[slotIdx].end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.first == pro && tmpPair.second == group;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsGrInSlot(int slotIdx,
        StudentGroup* group) {
    return find_if(slots[slotIdx].begin(), slots[slotIdx].end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.second == group;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsPrInSlot(int slotIdx,
        Professional* pro) {
    return find_if(slots[slotIdx].begin(), slots[slotIdx].end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.first == pro;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAs(Professional* pro,
        StudentGroup* group) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.first == pro & tmpPair.second == group;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsGr(StudentGroup* group) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.second == group;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsPr(Professional* pro) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.first == pro;
        });
}

bool isNbIntervByProReached(vector<int>& nbIntervByPr, Professional* pro) {
    return nbIntervByPr[pro->idx] >= 3;
}

bool isNbIntervByPrDaReached(vector<vector<int> >& nbIntervByPrDa, Professional* pro, TimeSlot* slot) {
    return nbIntervByPrDa[pro->idx][slot->day] >= 3;
}

bool isIntervPrSlAlready(vector<vector<bool> >& isIntervByPrSl, Professional* pro, TimeSlot* slot) {
    return isIntervByPrSl[pro->idx][slot->idx];
}

bool isNbIntervSlReached(vector<int>& nbIntervBySl, TimeSlot* slot) {
    return nbIntervBySl[slot->idx] >= 3;
}

bool isIntervGrSlAlready(vector<vector<bool> >& isIntervByGrSl, StudentGroup* group, TimeSlot* slot) {
    return isIntervByGrSl[group->idx][slot->idx];
}

std::vector<HeurNode*> iterate(Data* data, std::vector<HeurNode*>& nodes) {
    std::vector<HeurNode*> newNodes;
    // Generating mutations of assignations of all nodes
    for (auto& node : nodes) {
        // cout << " old cost " << node->cost << endl;
        vector<HeurNode*> mutatedNodes = node->generateMutationsAssignations(data);
        // Showing cost of new nodes
        newNodes.insert(newNodes.end(), mutatedNodes.begin(), mutatedNodes.end());
    }
    // // Generating mutations of groups of all nodes
    for (auto& node : nodes) {
        // cout << " old cost " << node->cost << endl;
        vector<HeurNode*> mutatedNodes = node->generateMutationsGroups(data);
        // Showing cost of new nodes
        newNodes.insert(newNodes.end(), mutatedNodes.begin(), mutatedNodes.end());
    }
    // Sorting new nodes by cost
    sort(newNodes.begin(), newNodes.end(),
        [] (HeurNode* node1, HeurNode* node2) {
            return node1->cost < node2->cost;
        });
    // Keeping only first 100 best nodes 
    vector<HeurNode*> nodesIteration(newNodes.begin(), newNodes.begin() + min(1ul, newNodes.size()));
    return nodesIteration;
}

HeurNode* pseudoGenetic(Data* data) {
    HeurNode* firstNode = firstFit(data);
    vector<HeurNode*> nodes { firstNode };
    float incumbent = firstNode->cost;
    int iteration = 0;
    bool isIterate = true;
    while (isIterate) {
        cout << " iteration " << iteration << endl;
        nodes = iterate(data, nodes);
        if (nodes.empty()) {
            cout << "ERROR: no nodes generated" << endl;
            nodes = { firstNode };
            break;
        }
        cout << nodes.front()->cost << " / " << incumbent << endl;
        if (nodes.front()->cost >= incumbent) isIterate = false;
        incumbent = nodes.front()->cost;
        iteration++;
        // if (iteration == 50) break;
    }
    cout << " nodes size " << nodes.size() << endl;
    return nodes[0];
}
