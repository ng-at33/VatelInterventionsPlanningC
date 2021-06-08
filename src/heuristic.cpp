/*
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.cpp
 */

#include <algorithm>
#include <set>
#include <utility>

#include "constants.h"
#include "data.h"
#include "heuristic.h"
#include "utils.h"

using namespace std;

int HeurNode::s_node_counter(0);

HeurNode::HeurNode(unique_ptr<Data>& p_data)
        : id(s_node_counter++), cost(0.0), is_interv_by_gr_sl(p_data->dimensions.num_groups,
          vector<bool>(p_data->dimensions.num_slots, false)),
          is_interv_by_pr_sl(p_data->dimensions.num_pros,
          vector<bool>(p_data->dimensions.num_slots, false)),
          nb_interv_by_pr_day(p_data->dimensions.num_pros,
          vector<int>(p_data->config.nb_days, 0)), nb_interv_by_pr(p_data->dimensions.num_pros, 0),
          nb_interv_by_gr(p_data->dimensions.num_groups, 0),
          nb_interv_by_sl(p_data->dimensions.num_slots, 0),
          nb_interv_by_day(p_data->config.nb_days, 0), slots(p_data->dimensions.num_slots,
          set<std::pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> >()), assignations() {};

HeurNode::HeurNode(std::unique_ptr<HeurNode>& p_node)
        : id(s_node_counter++), cost(p_node->cost), is_interv_by_gr_sl(p_node->is_interv_by_gr_sl),
          is_interv_by_pr_sl(p_node->is_interv_by_pr_sl),
          nb_interv_by_pr_day(p_node->nb_interv_by_pr_day),
          nb_interv_by_pr(p_node->nb_interv_by_pr), nb_interv_by_gr(p_node->nb_interv_by_gr),
          nb_interv_by_sl(p_node->nb_interv_by_sl), nb_interv_by_day(p_node->nb_interv_by_day),
          slots(p_node->slots), assignations(p_node->assignations) {};

ostream& HeurNode::print(ostream& os) const {
    os << "Node(id : " << id << ", cost : " << cost << ", slots : " << endl;
    auto cnt_slot = 0;
    for (auto const& p_slot : slots) {
        os << " " << cnt_slot << " : ";
        for (auto const& slot_cnt : p_slot) {
            os << "(" << (*slot_cnt.first).name << ", " << (*slot_cnt.second).name << ") ";
        }
        os << endl;
        cnt_slot++;
    }
    os << ")";
    return os;
};

void HeurNode::evaluate() {
    cost = 0.0;
    cost += computeSDVec(nb_interv_by_pr);
    cost += computeSDVec(nb_interv_by_gr);
    cost += computeSDVec(nb_interv_by_sl);
    cost += computeSDVec(nb_interv_by_day);
};

unique_ptr<HeurNode> firstFit(unique_ptr<Data>& p_data) {
    auto first_node = make_unique<HeurNode>(p_data);
    // Filling empty p_node
    // Sorting slots by least pros available
    auto slots_tmp = p_data->slots;
    sort(slots_tmp.begin(), slots_tmp.end(),
        [](shared_ptr<TimeSlot> p_slot1, shared_ptr<TimeSlot> p_slot2) {
            return p_slot1->pros.size() < p_slot2->pros.size();
        });

    // Iterating over sorted slots
    for (auto const& p_slot : slots_tmp) {
        // Sorting pros by least compatible
        auto pros = p_data->professionals;
        sort(pros.begin(), pros.end(),
            [] (shared_ptr<Professional> p_pro1, shared_ptr<Professional> p_pro2) {
                return p_pro1->slots.size() < p_pro2->slots.size();
            });
        for (auto it_pros = pros.begin(); it_pros != pros.end(); it_pros++) {
            for (auto it_group = p_data->groups.begin(); it_group != p_data->groups.end(); it_group++) {
                if (!first_node->isProAssignable(p_slot, *it_pros)) continue;
                if (!first_node->isGroupAssignable(*it_group, p_slot)) continue;
                if (!first_node->isProGroupAssignable(*it_pros, *it_group)) continue;
                if (!first_node->isSlotAssignable(p_slot)) continue;
                // Creating chosen <Professional*, StudenGroup*> pair
                pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> pairPrGr =
                    make_pair(*it_pros, *it_group);
                first_node->slots[p_slot->idx].insert(pairPrGr);
                first_node->assignations.insert(pairPrGr);
                first_node->is_interv_by_gr_sl[(*it_group)->idx][p_slot->idx] = true;
                first_node->is_interv_by_pr_sl[(*it_pros)->idx][p_slot->idx] = true;
                first_node->nb_interv_by_pr_day[(*it_pros)->idx][p_slot->day]++;
                first_node->nb_interv_by_pr[(*it_pros)->idx]++;
                first_node->nb_interv_by_gr[(*it_group)->idx]++;
                first_node->nb_interv_by_sl[p_slot->idx]++;
                first_node->nb_interv_by_day[p_slot->day]++;
            }
        }
    }
    first_node->evaluate();

    return first_node;
};

vector<unique_ptr<HeurNode>> HeurNode::generateSwaps(unique_ptr<Data>& p_data) {
    vector<unique_ptr<HeurNode>> swapped_nodes;
    auto slot_vec = this->slots.begin();
    for (int slot_idx = 0; slot_vec != this->slots.end(); slot_vec++, slot_idx++) {
        auto p_slot = p_data->slots[slot_idx];
        auto pair = slot_vec->begin();
        for (int pair_i = 0; pair != slot_vec->end(); pair++, pair_i++) {
            auto oslot_vec = this->slots.begin();
            for (int oslot_idx = 0; oslot_vec != this->slots.end(); oslot_vec++, oslot_idx++) {
                if (slot_idx != oslot_idx) {
                    auto p_oslot = p_data->slots[oslot_idx];
                    auto opair = oslot_vec->begin();
                    for (int opair_i = 0; opair != oslot_vec->end(); opair++, opair_i++) {
                        if (*opair == *pair) continue;
                        // Swap assignations if possible by creating a new p_node
                        // Checking if professionals can be swapped
                        if (pair->first != opair->first) {
                            if (!isProAssignable(p_oslot, pair->first)) continue;
                            if (!isProAssignable(p_slot, opair->first)) continue;
                        }
                        // Checking if groups can be swapped
                        if (pair->second != opair->second) {
                            if (!isGroupAssignable(pair->second, p_oslot)) continue;
                            if (!isGroupAssignable(opair->second, p_slot)) continue;
                        }
                        // Create new p_node with swapped pros and groups
                        auto swapped_node = make_unique<HeurNode>(*this);
                        swapped_node->is_interv_by_gr_sl[pair->second->idx][slot_idx] = false;
                        swapped_node->is_interv_by_gr_sl[opair->second->idx][oslot_idx] = false;
                        swapped_node->is_interv_by_gr_sl[pair->second->idx][oslot_idx] = true;
                        swapped_node->is_interv_by_gr_sl[opair->second->idx][slot_idx] = true;
                        swapped_node->is_interv_by_pr_sl[pair->first->idx][slot_idx] = false;
                        swapped_node->is_interv_by_pr_sl[opair->first->idx][oslot_idx] = false;
                        swapped_node->is_interv_by_pr_sl[pair->first->idx][oslot_idx] = true;
                        swapped_node->is_interv_by_pr_sl[opair->first->idx][slot_idx] = true;
                        swapped_node->nb_interv_by_pr_day[pair->first->idx][p_slot->day]--;
                        swapped_node->nb_interv_by_pr_day[opair->first->idx][p_oslot->day]--;
                        swapped_node->nb_interv_by_pr_day[pair->first->idx][p_oslot->day]++;
                        swapped_node->nb_interv_by_pr_day[opair->first->idx][p_slot->day]++;
                        auto assign1 = swapped_node->findAsInSlot(slot_idx,
                                                                 pair->first, pair->second);
                        auto assign2 = swapped_node->findAsInSlot(slot_idx, opair->first,
                                                                 opair->second);
                        swapped_node->slots[slot_idx].insert(*assign2);
                        swapped_node->slots[oslot_idx].insert(*assign1);
                        swapped_node->slots[slot_idx].erase(assign1);
                        swapped_node->slots[oslot_idx].erase(assign2);
                        swapped_node->evaluate();
                        swapped_nodes.push_back(std::move(swapped_node));
                    }
                }
            }
        }
    }
    return swapped_nodes;
}

vector<unique_ptr<HeurNode>> HeurNode::generateMutationsAssignations(unique_ptr<Data>& p_data) {
    vector<unique_ptr<HeurNode>> swapped_nodes;
    auto slot_vec = this->slots.begin();
    for (int slot_idx = 0; slot_vec != this->slots.end(); slot_vec++, slot_idx++) {
        auto p_slot = p_data->slots[slot_idx];
        auto pair = slot_vec->begin();
        for (int pair_i = 0; pair != slot_vec->end(); pair++, pair_i++) {
            auto oslot_vec = this->slots.begin();
            for (int oslot_idx = 0; oslot_vec != this->slots.end(); oslot_vec++, oslot_idx++) {
                if (slot_idx != oslot_idx) {
                    auto p_oslot = p_data->slots[oslot_idx];
                    if (!pair->first->isProAvailOnSlot(p_oslot)) continue;
                    if (isNbIntervByPrDaReached(nb_interv_by_pr_day, pair->first, p_oslot)) continue;
                    if (isIntervPrSlAlready(is_interv_by_pr_sl, pair->first, p_oslot)) continue;
                    if (isIntervGrSlAlready(is_interv_by_gr_sl, pair->second, p_oslot)) continue;
                    if (isNbIntervSlReached(nb_interv_by_sl, p_oslot)) continue;
                    // Create new p_node with assignation moved from p_slot to p_oslot
                    auto swapped_node = make_unique<HeurNode>(*this);
                    swapped_node->is_interv_by_gr_sl[pair->second->idx][slot_idx] = false;
                    swapped_node->is_interv_by_gr_sl[pair->second->idx][oslot_idx] = true;
                    swapped_node->is_interv_by_pr_sl[pair->first->idx][slot_idx] = false;
                    swapped_node->is_interv_by_pr_sl[pair->first->idx][oslot_idx] = true;
                    swapped_node->nb_interv_by_pr_day[pair->first->idx][p_slot->day]--;
                    swapped_node->nb_interv_by_pr_day[pair->first->idx][p_oslot->day]++;
                    swapped_node->nb_interv_by_sl[slot_idx]--;
                    swapped_node->nb_interv_by_sl[oslot_idx]++;
                    swapped_node->nb_interv_by_day[p_slot->day]--;
                    swapped_node->nb_interv_by_day[p_oslot->day]++;
                    auto assign = swapped_node->findAsInSlot(slot_idx, pair->first, pair->second);
                    swapped_node->slots[slot_idx].erase(assign);
                    swapped_node->slots[oslot_idx].insert(*assign);
                    swapped_node->evaluate();
                    swapped_nodes.push_back(std::move(swapped_node));
                }
            }
        }
    }
    return swapped_nodes;
}

vector<unique_ptr<HeurNode>> HeurNode::generateMutationsGroups(unique_ptr<Data>& p_data) {
    vector<unique_ptr<HeurNode>> swapped_nodes;
    auto slot_vec = this->slots.begin();
    for (int slot_idx = 0; slot_vec != this->slots.end(); slot_vec++, slot_idx++) {
        auto p_slot = p_data->slots[slot_idx];
        auto pair = slot_vec->begin();
        for (int pair_i = 0; pair != slot_vec->end(); pair++, pair_i++) {
            for (auto const& group : p_data->groups) {
                if (pair->second != group) {
                    if (isIntervGrSlAlready(is_interv_by_gr_sl, group, p_slot)) continue;
                    if (!isProGroupAssignable(pair->first, group)) continue;
                    // Create new p_node with group changed
                    auto swapped_node = make_unique<HeurNode>(*this);
                    swapped_node->is_interv_by_gr_sl[pair->second->idx][slot_idx] = false;
                    swapped_node->is_interv_by_gr_sl[group->idx][slot_idx] = true;
                    swapped_node->nb_interv_by_gr[pair->second->idx]--;
                    swapped_node->nb_interv_by_gr[group->idx]++;
                    auto assignSlot = swapped_node->findAsInSlot(slot_idx, pair->first, pair->second);
                    swapped_node->slots[slot_idx].erase(assignSlot);
                    std::pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> pairPrGr =
                        make_pair(pair->first, group);
                    swapped_node->slots[slot_idx].insert(pairPrGr);
                    auto assignAs = swapped_node->findAs(pair->first, pair->second);
                    swapped_node->assignations.erase(assignAs);
                    swapped_node->assignations.insert(pairPrGr);
                    swapped_node->evaluate();
                    swapped_nodes.push_back(std::move(swapped_node));
                }
            }
        }
    }
    return swapped_nodes;
}

bool HeurNode::isProAssignable(const shared_ptr<TimeSlot> p_slot,
                               const shared_ptr<Professional> p_pro) {
    if (!p_pro->isProAvailOnSlot(p_slot)) return false;
    if (isNbIntervByProReached(nb_interv_by_pr, p_pro)) return false;
    if (isNbIntervByPrDaReached(nb_interv_by_pr_day, p_pro, p_slot)) return false;
    if (isIntervPrSlAlready(is_interv_by_pr_sl, p_pro, p_slot)) return false;
    return true;
}

bool HeurNode::isSlotAssignable(const shared_ptr<TimeSlot> p_slot) {
    if (isNbIntervSlReached(nb_interv_by_sl, p_slot)) return false;
    return true;
}

bool HeurNode::isGroupAssignable(const shared_ptr<StudentGroup> p_group,
        const shared_ptr<TimeSlot> p_slot) {
    if (isIntervGrSlAlready(is_interv_by_gr_sl, p_group, p_slot)) return false;
    return true;
}

bool HeurNode::isProGroupAssignable(const shared_ptr<Professional> p_pro,
        const shared_ptr<StudentGroup> p_group) {
    // Check if <pro, group> is not already assigned
    auto tmp_pair = make_pair(p_pro, p_group);
    if (find(assignations.begin(), assignations.end(), tmp_pair) != assignations.end()) {
        return false;
    }
    if (!p_pro->isGroupCompatible(p_group)) return false;
    return true;
}

set<pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> >::iterator HeurNode::findAsInSlot(
        int slot_idx, const shared_ptr<Professional> p_pro, const shared_ptr<StudentGroup> p_group) {
    return find_if(slots[slot_idx].begin(), slots[slot_idx].end(),
        [&] (pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> tmp_pair) {
            return tmp_pair.first == p_pro && tmp_pair.second == p_group;
        });
}

set<pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> >::iterator HeurNode::findAsGrInSlot(
        int slot_idx,const shared_ptr<StudentGroup> p_group) {
    return find_if(slots[slot_idx].begin(), slots[slot_idx].end(),
        [&] (pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> tmp_pair) {
            return tmp_pair.second == p_group;
        });
}

set<pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> >::iterator HeurNode::findAsPrInSlot(
        int slot_idx, const shared_ptr<Professional> p_pro) {
    return find_if(slots[slot_idx].begin(), slots[slot_idx].end(),
        [&] (pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> tmp_pair) {
            return tmp_pair.first == p_pro;
        });
}

set<pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> >::iterator HeurNode::findAs(
        shared_ptr<Professional> p_pro, const shared_ptr<StudentGroup> p_group) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> tmp_pair) {
            return tmp_pair.first == p_pro & tmp_pair.second == p_group;
        });
}

set<pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> >::iterator HeurNode::findAsGr(
        shared_ptr<StudentGroup> p_group) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> tmp_pair) {
            return tmp_pair.second == p_group;
        });
}

set<pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> >::iterator HeurNode::findAsPr(
        shared_ptr<Professional> p_pro) {
    return find_if(assignations.begin(), assignations.end(),
        [&] (pair<shared_ptr<Professional>, shared_ptr<StudentGroup>> tmp_pair) {
            return tmp_pair.first == p_pro;
        });
}

bool isNbIntervByProReached(vector<int>& r_nb_interv_by_pr, shared_ptr<Professional> p_pro) {
    return r_nb_interv_by_pr[p_pro->idx] >= G_MAX_NUMBER_INTERV_PRO;
}

bool isNbIntervByPrDaReached(vector<vector<int> >& r_nb_interv_by_pr_day,
                             shared_ptr<Professional> p_pro, shared_ptr<TimeSlot> p_slot) {
    return r_nb_interv_by_pr_day[p_pro->idx][p_slot->day] >= G_MAX_NUMBER_INTERV_PRO_DAY;
}

bool isIntervPrSlAlready(vector<vector<bool> >& r_is_interv_by_pr_sl, shared_ptr<Professional> p_pro,
                         shared_ptr<TimeSlot> p_slot) {
    return r_is_interv_by_pr_sl[p_pro->idx][p_slot->idx];
}

bool isNbIntervSlReached(vector<int>& rnb_interv_by_sl, shared_ptr<TimeSlot> p_slot) {
    return rnb_interv_by_sl[p_slot->idx] >= G_MAX_NUMBER_INTERV_SLOT;
}

bool isIntervGrSlAlready(vector<vector<bool> >& r_is_interv_by_gr_sl,
                         shared_ptr<StudentGroup> p_group, shared_ptr<TimeSlot> p_slot) {
    return r_is_interv_by_gr_sl[p_group->idx][p_slot->idx];
}

vector<unique_ptr<HeurNode>> iterate(unique_ptr<Data>& p_data, vector<unique_ptr<HeurNode>>& r_nodes) {
    vector<unique_ptr<HeurNode>> new_nodes;
    // Generating mutations of assignations of all nodes
    for (auto& p_node : r_nodes) {
        // cout << " old cost " << p_node->cost << endl;
        vector<unique_ptr<HeurNode>> mutated_nodes = p_node->generateMutationsAssignations(p_data);
        // Showing cost of new nodes
        new_nodes.insert(new_nodes.end(), std::make_move_iterator(mutated_nodes.begin()),
                        std::make_move_iterator(mutated_nodes.end()));
    }
    // // Generating mutations of groups of all nodes
    for (auto& p_node : r_nodes) {
        // cout << " old cost " << p_node->cost << endl;
        vector<unique_ptr<HeurNode>> mutated_nodes = p_node->generateMutationsGroups(p_data);
        // Showing cost of new nodes
        new_nodes.insert(new_nodes.end(), std::make_move_iterator(mutated_nodes.begin()),
                        std::make_move_iterator(mutated_nodes.end()));
    }
    // Sorting new nodes by cost
    sort(new_nodes.begin(), new_nodes.end(),
        [](unique_ptr<HeurNode>& p_node1, unique_ptr<HeurNode>& p_node2) {
            return p_node1->cost < p_node2->cost;
        });
    // Keeping only first 100 best nodes
    vector<unique_ptr<HeurNode>> nodesIteration(std::make_move_iterator(new_nodes.begin()),
                                                std::make_move_iterator(new_nodes.begin()
                                                + min(1ul, new_nodes.size())));
    return nodesIteration;
}

unique_ptr<HeurNode> pseudoGenetic(unique_ptr<Data>& p_data) {
    auto p_first_node = firstFit(p_data);
    vector<unique_ptr<HeurNode>> nodes;
    float incumbent = p_first_node->cost;
    nodes.push_back(std::move(p_first_node));
    int iteration = 0;
    bool is_iterate = true;
    while (is_iterate) {
        cout << " iteration " << iteration << endl;
        nodes = iterate(p_data, nodes);
        if (nodes.empty()) {
            cout << "ERROR: no nodes generated" << endl;
            nodes.push_back(std::move(p_first_node));
            break;
        }
        cout << nodes.front()->cost << " / " << incumbent << endl;
        if (nodes.front()->cost >= incumbent) is_iterate = false;
        incumbent = nodes.front()->cost;
        iteration++;
        // if (iteration == 50) break;
    }
    cout << " nodes size " << nodes.size() << endl;
    return std::move(nodes[0]);
}
