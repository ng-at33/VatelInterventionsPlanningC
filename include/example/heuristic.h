/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.h: define a heuristic to solve the problem
 */


#pragma once

#include <set>
#include <utility>

#include "data.h"

struct HeurNode {
    static int NODE_COUNTER;
    int id;
    float cost;
    std::vector<std::set<std::pair<Professional*, StudentGroup*>>> slots; // Represent assigned <pro, student> pairs, indexed by slots
    HeurNode(Data& data); // Create an empty node
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const HeurNode& as) {
    return as.print(os);
};

HeurNode* firstFit(Data& data, HeurNode* node); // Complete node using first fit
HeurNode* firstFit(Data& data); // Create a solution using first fit