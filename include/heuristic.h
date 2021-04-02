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
    std::vector<int> nbIntervByDa; // Numer o9f interventions by <Day>
    std::vector<std::set<std::pair<Professional*, StudentGroup*>>> slots; // Represent assigned <pro, student> pairs, indexed by slots
    HeurNode(Data* data, float cost, std::vector<std::vector<bool> >& isIntervByGrSl,  // Create an fully filled node
        std::vector<std::vector<bool> >& isIntervByPrSl,
        std::vector<std::vector<int> >& nbIntervByPrDa,
        std::vector<int>& nbIntervByPr,
        std::vector<int>& nbIntervByGr,
        std::vector<int>& nbIntervBySl,
        std::vector<int>& nbIntervByDa,
        std::vector<std::set<std::pair<Professional*, StudentGroup*>>>& slots);
    std::ostream& print(std::ostream& os = std::cout) const;
};

// Overrides cout
inline std::ostream& operator<<(std::ostream& os, const HeurNode& as) {
    return as.print(os);
};

HeurNode* firstFit(std::vector<int>& nbIntervByPr, std::vector<int>& nbIntervByGr, // Create a solution using first fit
    std::vector<int>& nbIntervBySl, std::vector<int>& nbIntervByDa);

float evaluate(HeurNode* node); // Returns a measure of the fitness of this node
HeurNode* mutate(Data* data, HeurNode* node); // Make modifications on assignations to create a new, different node