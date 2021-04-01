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
    std::vector<std::vector<bool> > isIntervByGrSl; // Numer of interventions by <Group, Slot>
    std::vector<std::vector<bool> > isIntervByPrSl; // Numer of interventions by <Professional, Slot>
    std::vector<std::vector<int> > nbIntervByPrDa; // Numer of interventions by <Professional, Day>
    std::vector<int> nbIntervByPr; // Numer of interventions by <Professional>
    std::vector<int> nbIntervByGr; // Numer of interventions by <Group>
    std::vector<int> nbIntervBySl; // Numer of interventions by <Slot>
    std::vector<std::set<std::pair<Professional*, StudentGroup*>>> slots; // Represent assigned <pro, student> pairs, indexed by slots
    HeurNode(Data* data); // Create an empty node
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const HeurNode& as) {
    return as.print(os);
};

HeurNode* firstFit(Data* data, HeurNode* node); // Complete node using first fit
HeurNode* firstFit(Data* data); // Create a solution using first fit