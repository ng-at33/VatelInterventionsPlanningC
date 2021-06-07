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
    static int sNodeCounter;
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
    HeurNode(std::unique_ptr<Data>& data); // Create an empty node
    HeurNode(std::unique_ptr<HeurNode>& node); // Create a copy of a node
    std::vector<std::unique_ptr<HeurNode>> generateSwaps(std::unique_ptr<Data>& data); // Generate all nodes with interventions swapped
    std::vector<std::unique_ptr<HeurNode>> generateMutationsAssignations(std::unique_ptr<Data>& data); // Generate all nodes with interventions swapped
    std::vector<std::unique_ptr<HeurNode>> generateMutationsGroups(std::unique_ptr<Data>& data); // Generate all nodes with interventions swapped
    void evaluate(); // Set the cost for this node
    bool isSlotAssignable(TimeSlot* pSlot);  // Returns true if slot is assignable
    bool isProAssignable(TimeSlot* pSlot, Professional* pPro);  // Returns true if professional can be assigned to slot with idx slotIdx
    bool isGroupAssignable(StudentGroup* pGroup, TimeSlot* pSlot);  // Returns true if students group can be assigned to slot with idx slotIdx
    bool isProGroupAssignable(Professional* pPro, StudentGroup* pGroup);  // Returns true if pro can be assigned to student group
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsInSlot(int slotIdx, Professional* pPro, StudentGroup* pGroup); // Returns pointer to assignation with <pro, group> in slot
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsGrInSlot(int slotIdx, StudentGroup* pGroup); // Returns pointer to assignation with group in slot
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsPrInSlot(int slotIdx, Professional* pPro); // Returns pointer to assignation with pro in slot
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAs(Professional* pPro, StudentGroup* pGroup); // Returns pointer to assignation with <pro, group> in assignations
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsGr(StudentGroup* pGroup); // Returns pointer to assignation with group in assignations
    std::set<std::pair<Professional*, StudentGroup*> >::iterator findAsPr(Professional* pPro); // Returns pointer to assignation with pro in assignations
    std::ostream& print(std::ostream& os = std::cout) const;
};
// Override cout
inline std::ostream& operator<<(std::ostream& os, const HeurNode& rNode) { return rNode.print(os); };
// Returns a measure of the fitness of this node
// float evaluate(std::vector<int>& nbIntervByPr,
//     std::vector<int>& nbIntervByGr, std::vector<int>& nbIntervBySl, std::vector<int>& nbIntervByDa);
bool isNbIntervByProReached(std::vector<int>& nbIntervByPr, Professional* pPro); // Return true if number of interventions for this professional has been reached
bool isNbIntervByPrDaReached(std::vector<std::vector<int> >& rNbIntervByPrDa, Professional* pPro, TimeSlot* pSlot); // Return true if number of interventions for this professional on this day has been reached
bool isIntervPrSlAlready(std::vector<std::vector<bool> >& rIsIntervByPrSl, Professional* pPro, TimeSlot* pSlot); // Return true if this pro has already been assigned on this slot
bool isNbIntervSlReached(std::vector<int>& rNbIntervBySl, TimeSlot* pSlot); // Return true if number of interventions for this slot has been reached
bool isIntervGrSlAlready(std::vector<std::vector<bool> >& rIsIntervByGrSl, StudentGroup* pGroup, TimeSlot* pSlot); // Return true if this group has already been assigned on this slot
// Create a solution using first fit
std::unique_ptr<HeurNode> firstFit(std::unique_ptr<Data>& pData);
// Iterate to select and generate swaps of nodes
std::vector<std::unique_ptr<HeurNode>> iterate(std::unique_ptr<Data>& pData, std::vector<std::unique_ptr<HeurNode>>& rNodes);
// Launch the full algorithm, return the best node
std::unique_ptr<HeurNode> pseudoGenetic(std::unique_ptr<Data>& pData);