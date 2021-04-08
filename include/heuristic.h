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
    std::vector<int> nbIntervByDa; // Numer of interventions by <Day>
    std::vector<std::set<std::pair<Professional*, StudentGroup*> > > slots; // Represent assigned <pro, student> pairs, indexed by slots
    std::set<std::pair<Professional*, StudentGroup*> > assignations; // Contain pairs of assigned <pro, students> to look for already assigned pairs
    HeurNode(float cost, std::vector<std::vector<bool> >& isIntervByGrSl,  // Create a fully filled node
        std::vector<std::vector<bool> >& isIntervByPrSl,
        std::vector<std::vector<int> >& nbIntervByPrDa, std::vector<int>& nbIntervByPr,
        std::vector<int>& nbIntervByGr, std::vector<int>& nbIntervBySl,
        std::vector<int>& nbIntervByDa,
        std::vector<std::set<std::pair<Professional*, StudentGroup*> > >& slots,
        std::set<std::pair<Professional*, StudentGroup*> >& assignations);
    std::vector<HeurNode*> generateSwaps(Data* data); // Generate all nodes with interventions swapped
    std::vector<HeurNode*> generateMutationsAssignations(Data* data); // Generate all nodes with interventions swapped
    std::vector<HeurNode*> generateMutationsGroups(Data* data); // Generate all nodes with interventions swapped
    bool isProAssignable(Data* data, int slotIdx, Professional* pro);  // Returns true if professional can be assigned to slot with idx slotIdx
    bool isGroupAssignable(int slotIdx, StudentGroup* group);  // Returns true if students group can be assigned to slot with idx slotIdx
    bool isProGroupAssignable(Professional* pro, StudentGroup* group);  // Returns true if pro can be assigned to student group
    std::ostream& print(std::ostream& os = std::cout) const;
};
// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const HeurNode& as) { return as.print(os); };
// Returns a measure of the fitness of this node
float evaluate(std::vector<int>& nbIntervByPr,
    std::vector<int>& nbIntervByGr, std::vector<int>& nbIntervBySl, std::vector<int>& nbIntervByDa);
// Create a solution using first fit
HeurNode* firstFit(Data* data);
// Iterate to select and generate swaps of nodes
std::vector<HeurNode*> iterate(Data* data, std::vector<HeurNode*>& nodes);
// Launch the full algorithm, return the best node
HeurNode* pseudoGenetic(Data* data);