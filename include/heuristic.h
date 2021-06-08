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
    static int s_node_counter;
    int id;
    float cost;
    std::vector<std::vector<bool> > is_interv_by_gr_sl; // True if intervention on <Group, Slot>
    std::vector<std::vector<bool> > is_interv_by_pr_sl; // True if intervention on <Professional, Slot>
    std::vector<std::vector<int> > nb_interv_by_pr_day; // Number of interventions by <Professional, Day>
    std::vector<int> nb_interv_by_pr; // Number of interventions by <Professional>
    std::vector<int> nb_interv_by_gr; // Number of interventions by <Group>
    std::vector<int> nb_interv_by_sl; // Number of interventions by <Slot>
    std::vector<int> nb_interv_by_day; // Number of interventions by <Day>
    std::vector<std::set<std::pair<std::shared_ptr<Professional>, std::shared_ptr<StudentGroup>> > > slots; // Represent assigned <pro, student> pairs, indexed by slots
    std::set<std::pair<std::shared_ptr<Professional>, std::shared_ptr<StudentGroup>> > assignations; // Contain pairs of assigned <pro, students> to look for already assigned pairs
    HeurNode(std::unique_ptr<Data>& data); // Create an empty node
    HeurNode(std::unique_ptr<HeurNode>& node); // Create a copy of a node
    std::vector<std::unique_ptr<HeurNode>> generateSwaps(std::unique_ptr<Data>& data); // Generate all nodes with interventions swapped
    std::vector<std::unique_ptr<HeurNode>> generateMutationsAssignations(
        std::unique_ptr<Data>& data); // Generate all nodes with interventions swapped
    std::vector<std::unique_ptr<HeurNode>> generateMutationsGroups(std::unique_ptr<Data>& data); // Generate all nodes with interventions swapped
    void evaluate(); // Set the cost for this node
    bool isSlotAssignable(const std::shared_ptr<TimeSlot> p_slot);  // Returns true if slot is assignable
    bool isProAssignable(const std::shared_ptr<TimeSlot> p_slot,
                         const std::shared_ptr<Professional> p_pro);  // Returns true if professional can be assigned to slot with idx slot_idx
    bool isGroupAssignable(const std::shared_ptr<StudentGroup> p_group,
                           const std::shared_ptr<TimeSlot> p_slot);  // Returns true if students group can be assigned to slot with idx slot_idx
    bool isProGroupAssignable(const std::shared_ptr<Professional> p_pro,
                              const std::shared_ptr<StudentGroup> p_group);  // Returns true if pro can be assigned to student group
    std::set<std::pair<std::shared_ptr<Professional>, std::shared_ptr<StudentGroup>> >::iterator
        findAsInSlot(int slot_idx, const std::shared_ptr<Professional> p_pro,
                     const std::shared_ptr<StudentGroup> p_group); // Returns pointer to assignation with <pro, group> in slot
    std::set<std::pair<std::shared_ptr<Professional>, std::shared_ptr<StudentGroup>> >::iterator
        findAsGrInSlot(int slot_idx, const std::shared_ptr<StudentGroup> p_group); // Returns pointer to assignation with group in slot
    std::set<std::pair<std::shared_ptr<Professional>, std::shared_ptr<StudentGroup>> >::iterator
        findAsPrInSlot(int slot_idx, const std::shared_ptr<Professional> p_pro); // Returns pointer to assignation with pro in slot
    std::set<std::pair<std::shared_ptr<Professional>, std::shared_ptr<StudentGroup>> >::iterator
        findAs(const std::shared_ptr<Professional> p_pro, const std::shared_ptr<StudentGroup> p_group); // Returns pointer to assignation with <pro, group> in assignations
    std::set<std::pair<std::shared_ptr<Professional>, std::shared_ptr<StudentGroup>> >::iterator
        findAsGr(const std::shared_ptr<StudentGroup> p_group); // Returns pointer to assignation with group in assignations
    std::set<std::pair<std::shared_ptr<Professional>, std::shared_ptr<StudentGroup>> >::iterator
        findAsPr(const std::shared_ptr<Professional> p_pro); // Returns pointer to assignation with pro in assignations
    std::ostream& print(std::ostream& os = std::cout) const;
};
// Override cout
inline std::ostream& operator<<(std::ostream& os, const HeurNode& r_node) { return r_node.print(os); };
// Returns a measure of the fitness of this node
// float evaluate(std::vector<int>& nb_interv_by_pr,
//     std::vector<int>& nbIntervByGr, std::vector<int>& nbIntervBySl, std::vector<int>& nbIntervByDa);
bool isNbIntervByProReached(std::vector<int>& r_nb_interv_by_pr,
                            const std::shared_ptr<Professional> p_pro); // Return true if number of interventions for this professional has been reached
bool isNbIntervByPrDaReached(std::vector<std::vector<int> >& r_nb_interv_by_pr_day,
                             const std::shared_ptr<Professional> p_pro,
                             const std::shared_ptr<TimeSlot> p_slot); // Return true if number of interventions for this professional on this day has been reached
bool isIntervPrSlAlready(std::vector<std::vector<bool> >& r_is_interv_by_pr_sl,
                         const std::shared_ptr<Professional> p_pro,
                         const std::shared_ptr<TimeSlot> p_slot); // Return true if this pro has already been assigned on this slot
bool isNbIntervSlReached(std::vector<int>& r_nb_interv_by_sl, const std::shared_ptr<TimeSlot> p_slot); // Return true if number of interventions for this slot has been reached
bool isIntervGrSlAlready(std::vector<std::vector<bool> >& r_is_interv_by_gr_sl,
                         const std::shared_ptr<StudentGroup> p_group,
                         const std::shared_ptr<TimeSlot> p_slot); // Return true if this group has already been assigned on this slot
// Create a solution using first fit
std::unique_ptr<HeurNode> firstFit(std::unique_ptr<Data>& p_data);
// Iterate to select and generate swaps of nodes
std::vector<std::unique_ptr<HeurNode>> iterate(std::unique_ptr<Data>& p_data,
                                               std::vector<std::unique_ptr<HeurNode>>& r_nodes);
// Launch the full algorithm, return the best node
std::unique_ptr<HeurNode> pseudoGenetic(std::unique_ptr<Data>& p_data);