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
    std::vector<int> nbIntervByPr; // Number of interventions by <Professional>
    std::vector<int> nbIntervByGr; // Number of interventions by <Group>
    std::vector<int> nbIntervBySl; // Number of interventions by <Slot>
    std::vector<int> nbIntervByDa; // Number of interventions by <Day>
    std::vector<std::set<std::pair<Professional*, StudentGroup*> > > slots; // Represent assigned <pro, student> pairs, indexed by slots
    std::set<std::pair<Professional*, StudentGroup*> > assignations; // Contain pairs of assigned <pro, students> to look for already assigned pairs
    HeurNode(Data* data); // Create an empty node
    HeurNode(HeurNode* node); // Create a copy of a node
    std::vector<HeurNode*> generateSwaps(Data* data); // Generate all nodes with interventions swapped
    std::vector<HeurNode*> generateMutationsAssignations(Data* data); // Generate all nodes with interventions swapped
    std::vector<HeurNode*> generateMutationsGroups(Data* data); // Generate all nodes with interventions swapped
    void evaluate(); // Set the cost for this node
    bool isSlotAssignable(TimeSlot* slot);  // Returns true if slot is assignable
    bool isProAssignable(Data* data, TimeSlot* slot, Professional* pro);  // Returns true if professional can be assigned to slot with idx slotIdx
    bool isGroupAssignable(StudentGroup* group, TimeSlot* slot);  // Returns true if students group can be assigned to slot with idx slotIdx
    bool isProGroupAssignable(Professional* pro, StudentGroup* group);  // Returns true if pro can be assigned to student group
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsInSlot(int slotIdx, Professional* pro, StudentGroup* group); // Returns pointer to assignation with <pro, group> in slot
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsGrInSlot(int slotIdx, StudentGroup* group); // Returns pointer to assignation with <pro, group> in slot
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsPrInSlot(int slotIdx, Professional* pro); // Returns pointer to assignation with <pro, group> in slot
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAs(Professional* pro, StudentGroup* group); // Returns pointer to assignation with <pro, group> in assignations
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsGr(StudentGroup* group); // Returns pointer to assignation with <pro, group> in assignations
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsPr(Professional* pro); // Returns pointer to assignation with <pro, group> in assignations
    std::ostream& print(std::ostream& os = std::cout) const;
};
// Override cout
inline std::ostream& operator<<(std::ostream& os, const HeurNode& as) { return as.print(os); };
// Returns a measure of the fitness of this node
// float evaluate(std::vector<int>& nbIntervByPr,
//     std::vector<int>& nbIntervByGr, std::vector<int>& nbIntervBySl, std::vector<int>& nbIntervByDa);
bool isNbIntervByProReached(std::vector<int>& nbIntervByPr, Professional* pro); // Return true if number of interventions for this professional has been reached
bool isNbIntervByPrDaReached(std::vector<std::vector<int> >& nbIntervByPrDa, Professional* pro, TimeSlot* slot); // Return true if number of interventions for this professional has been reached
bool isIntervPrSlAlready(std::vector<std::vector<bool> >& isIntervByPrSl, Professional* pro, TimeSlot* slot); // Return true if number of interventions for this professional has been reached
bool isNbIntervSlReached(std::vector<int>& nbIntervBySl, TimeSlot* slot); // Return true if number of interventions for this professional has been reached
bool isIntervGrSlAlready(std::vector<std::vector<bool> >& isIntervByGrSl, StudentGroup* group, TimeSlot* slot); // Return true if number of interventions for this professional has been reached
// Create a solution using first fit
HeurNode* firstFit(Data* data);
// Iterate to select and generate swaps of nodes
std::vector<HeurNode*> iterate(Data* data, std::vector<HeurNode*>& nodes);
// Launch the full algorithm, return the best node
HeurNode* pseudoGenetic(Data* data);