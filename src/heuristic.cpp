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

int HeurNode::sNodeCounter(0);

HeurNode::HeurNode(Data* pData)
        : id(sNodeCounter++), cost(0.0), isIntervByGrSl(pData->dimensions.numGroups, vector<bool>(pData->dimensions.numSlots, false)),
        isIntervByPrSl(pData->dimensions.numPros, vector<bool>(pData->dimensions.numSlots, false)),
        nbIntervByPrDa(pData->dimensions.numPros, vector<int>(pData->config .nbDays, 0)),
        nbIntervByPr(pData->dimensions.numPros, 0),
        nbIntervByGr(pData->dimensions.numGroups, 0),
        nbIntervBySl(pData->dimensions.numSlots, 0),
        nbIntervByDa(pData->config.nbDays, 0),
        slots(pData->dimensions.numSlots, set<pair<Professional*, StudentGroup*> >()),
        assignations() {};

HeurNode::HeurNode(HeurNode* pNode)
        : id(sNodeCounter++), cost(pNode->cost), isIntervByGrSl(pNode->isIntervByGrSl),
        isIntervByPrSl(pNode->isIntervByPrSl), nbIntervByPrDa(pNode->nbIntervByPrDa),
        nbIntervByPr(pNode->nbIntervByPr), nbIntervByGr(pNode->nbIntervByGr),
        nbIntervBySl(pNode->nbIntervBySl), nbIntervByDa(pNode->nbIntervByDa),
        slots(pNode->slots), assignations(pNode->assignations) {};

ostream& HeurNode::print(ostream& os) const {
    os << "Node(id : " << id << ", cost : " << cost << ", slots : " << endl;
    auto cntSlot = 0;
    for (auto const& pSlot : slots) {
        os << " " << cntSlot << " : ";
        for (auto const& slotCnt : pSlot) {
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

HeurNode* firstFit(Data* pData) {
    auto* firstNode = new HeurNode(pData);
    // Filling empty pNode
    // Sorting slots by least pros available
    auto slotsTmp = pData->slots;
    sort(slotsTmp.begin(), slotsTmp.end(),
        [] (TimeSlot* pSlot1, TimeSlot* pSlot2) {
            return pSlot1->pros.size() < pSlot2->pros.size();
        });

    // Iterating over sorted slots
    for (auto const& pSlot : slotsTmp) {
        // Sorting pros by least compatible
        auto pros = pData->professionals;
        sort(pros.begin(), pros.end(),
            [] (Professional* pPro1, Professional* pPro2) {
                return pPro1->slots.size() < pPro2->slots.size();
            });
        for (auto itPros = pros.begin(); itPros != pros.end(); itPros++) {
            for (auto itGroup = pData->groups.begin(); itGroup != pData->groups.end(); itGroup++) {
                if (!firstNode->isProAssignable(pSlot, *itPros)) continue;
                if (!firstNode->isGroupAssignable(*itGroup, pSlot)) continue;
                if (!firstNode->isProGroupAssignable(*itPros, *itGroup)) continue;
                if (!firstNode->isSlotAssignable(pSlot)) continue;
                // Creating chosen <Professional*, StudenGroup*> pair
                pair<Professional*, StudentGroup*> pairPrGr = make_pair(*itPros, *itGroup);
                firstNode->slots[pSlot->idx].insert(pairPrGr);
                firstNode->assignations.insert(pairPrGr);
                firstNode->isIntervByGrSl[(*itGroup)->idx][pSlot->idx] = true;
                firstNode->isIntervByPrSl[(*itPros)->idx][pSlot->idx] = true;
                firstNode->nbIntervByPrDa[(*itPros)->idx][pSlot->day]++;
                firstNode->nbIntervByPr[(*itPros)->idx]++;
                firstNode->nbIntervByGr[(*itGroup)->idx]++;
                firstNode->nbIntervBySl[pSlot->idx]++;
                firstNode->nbIntervByDa[pSlot->day]++;
            }
        }
    }
    firstNode->evaluate();

    return firstNode;
};

vector<HeurNode*> HeurNode::generateSwaps(Data* pData) {
    std::vector<HeurNode*> swappedNodes;
    auto slotVec = this->slots.begin();
    for (int slotIdx = 0; slotVec != this->slots.end(); slotVec++, slotIdx++) {
        TimeSlot* pSlot = pData->slots[slotIdx];
        auto pair = slotVec->begin();
        for (int pairI = 0; pair != slotVec->end(); pair++, pairI++) {
            auto oslotVec = this->slots.begin();
            for (int oslotIdx = 0; oslotVec != this->slots.end(); oslotVec++, oslotIdx++) {
                if (slotIdx != oslotIdx) {
                    TimeSlot* pOslot = pData->slots[oslotIdx];
                    auto opair = oslotVec->begin();
                    for (int opairI = 0; opair != oslotVec->end(); opair++, opairI++) {
                        if (*opair == *pair) continue;
                        // Swap assignations if possible by creating a new pNode
                        // Checking if professionals can be swapped
                        if (pair->first != opair->first) {
                            if (!isProAssignable(pOslot, pair->first)) continue;
                            if (!isProAssignable(pSlot, opair->first)) continue;
                        }
                        // Checking if groups can be swapped
                        if (pair->second != opair->second) {
                            if (!isGroupAssignable(pair->second, pOslot)) continue;
                            if (!isGroupAssignable(opair->second, pSlot)) continue;
                        }
                        // Create new pNode with swapped pros and groups
                        HeurNode* swappedNode = new HeurNode(this);
                        swappedNode->isIntervByGrSl[pair->second->idx][slotIdx] = false;
                        swappedNode->isIntervByGrSl[opair->second->idx][oslotIdx] = false;
                        swappedNode->isIntervByGrSl[pair->second->idx][oslotIdx] = true;
                        swappedNode->isIntervByGrSl[opair->second->idx][slotIdx] = true;
                        swappedNode->isIntervByPrSl[pair->first->idx][slotIdx] = false;
                        swappedNode->isIntervByPrSl[opair->first->idx][oslotIdx] = false;
                        swappedNode->isIntervByPrSl[pair->first->idx][oslotIdx] = true;
                        swappedNode->isIntervByPrSl[opair->first->idx][slotIdx] = true;
                        swappedNode->nbIntervByPrDa[pair->first->idx][pSlot->day]--;
                        swappedNode->nbIntervByPrDa[opair->first->idx][pOslot->day]--;
                        swappedNode->nbIntervByPrDa[pair->first->idx][pOslot->day]++;
                        swappedNode->nbIntervByPrDa[opair->first->idx][pSlot->day]++;
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

vector<HeurNode*> HeurNode::generateMutationsAssignations(Data* pData) {
    std::vector<HeurNode*> swappedNodes;
    auto slotVec = this->slots.begin();
    for (int slotIdx = 0; slotVec != this->slots.end(); slotVec++, slotIdx++) {
        TimeSlot* pSlot = pData->slots[slotIdx];
        auto pair = slotVec->begin();
        for (int pairI = 0; pair != slotVec->end(); pair++, pairI++) {
            auto oslotVec = this->slots.begin();
            for (int oslotIdx = 0; oslotVec != this->slots.end(); oslotVec++, oslotIdx++) {
                if (slotIdx != oslotIdx) {
                    TimeSlot* pOslot = pData->slots[oslotIdx];
                    if (!pair->first->isProAvailOnSlot(pOslot)) continue;
                    if (isNbIntervByPrDaReached(nbIntervByPrDa, pair->first, pOslot)) continue;
                    if (isIntervPrSlAlready(isIntervByPrSl, pair->first, pOslot)) continue;
                    if (isIntervGrSlAlready(isIntervByGrSl, pair->second, pOslot)) continue;
                    if (isNbIntervSlReached(nbIntervBySl, pOslot)) continue;
                    // Create new pNode with assignation moved from pSlot to pOslot
                    HeurNode* swappedNode = new HeurNode(this);
                    swappedNode->isIntervByGrSl[pair->second->idx][slotIdx] = false;
                    swappedNode->isIntervByGrSl[pair->second->idx][oslotIdx] = true;
                    swappedNode->isIntervByPrSl[pair->first->idx][slotIdx] = false;
                    swappedNode->isIntervByPrSl[pair->first->idx][oslotIdx] = true;
                    swappedNode->nbIntervByPrDa[pair->first->idx][pSlot->day]--;
                    swappedNode->nbIntervByPrDa[pair->first->idx][pOslot->day]++;
                    swappedNode->nbIntervBySl[slotIdx]--;
                    swappedNode->nbIntervBySl[oslotIdx]++;
                    swappedNode->nbIntervByDa[pSlot->day]--;
                    swappedNode->nbIntervByDa[pOslot->day]++;
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

vector<HeurNode*> HeurNode::generateMutationsGroups(Data* pData) {
    std::vector<HeurNode*> swappedNodes;
    auto slotVec = this->slots.begin();
    for (int slotIdx = 0; slotVec != this->slots.end(); slotVec++, slotIdx++) {
        auto pSlot = pData->slots[slotIdx];
        auto pair = slotVec->begin();
        for (int pairI = 0; pair != slotVec->end(); pair++, pairI++) {
            for (auto const& group : pData->groups) {
                if (pair->second != group) {
                    if (isIntervGrSlAlready(isIntervByGrSl, group, pSlot)) continue;
                    if (!isProGroupAssignable(pair->first, group)) continue;
                    // Create new pNode with group changed
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

bool HeurNode::isProAssignable(TimeSlot* pSlot, Professional* pPro) {
    if (!pPro->isProAvailOnSlot(pSlot)) return false;
    if (isNbIntervByProReached(nbIntervByPr, pPro)) return false;
    if (isNbIntervByPrDaReached(nbIntervByPrDa, pPro, pSlot)) return false;
    if (isIntervPrSlAlready(isIntervByPrSl, pPro, pSlot)) return false;
    return true;
}

bool HeurNode::isSlotAssignable(TimeSlot* pSlot) {
    if (isNbIntervSlReached(nbIntervBySl, pSlot)) return false;
    return true;
}

bool HeurNode::isGroupAssignable(StudentGroup* pGroup, TimeSlot* pSlot) {
    if (isIntervGrSlAlready(isIntervByGrSl, pGroup, pSlot)) return false;
    return true;
}

bool HeurNode::isProGroupAssignable(Professional* pPro, StudentGroup* pGroup) {
    // Check if <pro, group> is not already assigned
    auto tmpPair = make_pair(pPro, pGroup);
    if (find(assignations.begin(), assignations.end(), tmpPair) != assignations.end()) {
        return false;
    }
    if (!pPro->isGroupCompatible(pGroup)) return false;
    return true;
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsInSlot(int slotIdx,
        Professional* pPro, StudentGroup* pGroup) {
    return find_if(slots[slotIdx].begin(), slots[slotIdx].end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.first == pPro && tmpPair.second == pGroup;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsGrInSlot(int slotIdx,
        StudentGroup* pGroup) {
    return find_if(slots[slotIdx].begin(), slots[slotIdx].end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.second == pGroup;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsPrInSlot(int slotIdx,
        Professional* pPro) {
    return find_if(slots[slotIdx].begin(), slots[slotIdx].end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.first == pPro;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAs(Professional* pPro,
        StudentGroup* pGroup) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.first == pPro & tmpPair.second == pGroup;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsGr(StudentGroup* pGroup) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.second == pGroup;
        });
}

std::set<std::pair<Professional*, StudentGroup*> >::iterator HeurNode::findAsPr(Professional* pPro) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (std::pair<Professional*, StudentGroup*> tmpPair) {
            return tmpPair.first == pPro;
        });
}

bool isNbIntervByProReached(vector<int>& rNbIntervByPr, Professional* pPro) {
    return rNbIntervByPr[pPro->idx] >= 3;
}

bool isNbIntervByPrDaReached(vector<vector<int> >& rNbIntervByPrDa, Professional* pPro, TimeSlot* pSlot) {
    return rNbIntervByPrDa[pPro->idx][pSlot->day] >= 3;
}

bool isIntervPrSlAlready(vector<vector<bool> >& rIsIntervByPrSl, Professional* pPro, TimeSlot* pSlot) {
    return rIsIntervByPrSl[pPro->idx][pSlot->idx];
}

bool isNbIntervSlReached(vector<int>& rNbIntervBySl, TimeSlot* pSlot) {
    return rNbIntervBySl[pSlot->idx] >= 3;
}

bool isIntervGrSlAlready(vector<vector<bool> >& rIsIntervByGrSl, StudentGroup* pGroup, TimeSlot* pSlot) {
    return rIsIntervByGrSl[pGroup->idx][pSlot->idx];
}

std::vector<HeurNode*> iterate(Data* pData, std::vector<HeurNode*>& rNodes) {
    std::vector<HeurNode*> newNodes;
    // Generating mutations of assignations of all nodes
    for (auto& pNode : rNodes) {
        // cout << " old cost " << pNode->cost << endl;
        vector<HeurNode*> mutatedNodes = pNode->generateMutationsAssignations(pData);
        // Showing cost of new nodes
        newNodes.insert(newNodes.end(), mutatedNodes.begin(), mutatedNodes.end());
    }
    // // Generating mutations of groups of all nodes
    for (auto& pNode : rNodes) {
        // cout << " old cost " << pNode->cost << endl;
        vector<HeurNode*> mutatedNodes = pNode->generateMutationsGroups(pData);
        // Showing cost of new nodes
        newNodes.insert(newNodes.end(), mutatedNodes.begin(), mutatedNodes.end());
    }
    // Sorting new nodes by cost
    sort(newNodes.begin(), newNodes.end(),
        [] (HeurNode* pNode1, HeurNode* pNode2) {
            return pNode1->cost < pNode2->cost;
        });
    // Keeping only first 100 best nodes 
    vector<HeurNode*> nodesIteration(newNodes.begin(), newNodes.begin() + min(1ul, newNodes.size()));
    return nodesIteration;
}

HeurNode* pseudoGenetic(Data* pData) {
    HeurNode* pFirstNode = firstFit(pData);
    vector<HeurNode*> nodes { pFirstNode };
    float incumbent = pFirstNode->cost;
    int iteration = 0;
    bool isIterate = true;
    while (isIterate) {
        cout << " iteration " << iteration << endl;
        nodes = iterate(pData, nodes);
        if (nodes.empty()) {
            cout << "ERROR: no nodes generated" << endl;
            nodes = { pFirstNode };
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
