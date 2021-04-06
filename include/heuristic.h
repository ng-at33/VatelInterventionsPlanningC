/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.h: define a heuristic to solve the problem
 */


#pragma once

#include <set>
#include <utility>

#include "data.h"

// Fully hold a partial or complete heuristic solution
struct HeurNode {
    static int NODE_COUNTER;
    int id;
    float cost;
    std::vector<std::vector<bool> > isIntervByGrSl; // True if intervention on <Group, Slot>
    std::vector<std::vector<bool> > isIntervByPrSl; // True if intervention on <Professional, Slot>
    std::vector<std::vector<int> > nbIntervByPrDa; // Number of interventions by <Professional, Day>
    std::vector<int> nbIntervByPr; // Numer of interventions by <Professional>
    std::vector<int> nbIntervByGr; // Numer of interventions by <Group>
    std::vector<int> nbIntervBySl; // Numer of interventions by <Slot>
    std::vector<int> nbIntervByDa; // Numer o9f interventions by <Day>
    std::vector<std::set<std::pair<Professional*, StudentGroup*> > > slots; // Represent assigned <pro, student> pairs, indexed by slots
    std::set<std::pair<Professional*, StudentGroup*> > assignations; // Contain pairs of assigned <pro, students> to look for already assigned pairs
    HeurNode(Data* data, float cost, std::vector<std::vector<bool> >& isIntervByGrSl,  // Create an fully filled node
        std::vector<std::vector<bool> >& isIntervByPrSl,
        std::vector<std::vector<int> >& nbIntervByPrDa, std::vector<int>& nbIntervByPr,
        std::vector<int>& nbIntervByGr, std::vector<int>& nbIntervBySl,
        std::vector<int>& nbIntervByDa,
        std::vector<std::set<std::pair<Professional*, StudentGroup*> > >& slots,
        std::set<std::pair<Professional*, StudentGroup*> >& assignations);
    std::vector<HeurNode*> generateSwaps(Data* data); // Generate all nodes with interventions swapped
    bool isProSwappable(Data* data, int slotIdx, Professional* pro);  // Returns true if professional can be swapped to slot with idx slotIdx
    bool isGroupSwappable(int slotIdx, StudentGroup* group);  // Returns true if students group can be swapped to slot with idx slotIdx
    std::ostream& print(std::ostream& os = std::cout) const;
};
// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const HeurNode& as) { return as.print(os); };
// Returns a measure of the fitness of this node
float evaluate(Data* data, std::vector<bool>& isIntervByGrSl, std::vector<int>& nbIntervByPr,
    std::vector<int>& nbIntervByGr, std::vector<int>& nbIntervBySl, std::vector<int>& nbIntervByDa);
// Create a solution using first fit
HeurNode* firstFit(Data* data);
// Iterate to select and generate swaps of nodes
std::vector<HeurNode*> iterate(Data* data, std::vector<HeurNode*> nodes);