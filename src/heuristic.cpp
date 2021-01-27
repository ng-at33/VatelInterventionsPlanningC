/* 
 * Developped by Alexis TOULLAT (alexis.toullat@at-consulting.fr)
 * heuristic.cpp
 */

#include "data.h"
#include "heuristic.h"

using namespace std;

int HeurNode::NODE_COUNTER(0);

HeurNode::HeurNode(Data& data) : id(NODE_COUNTER++), cost(0),
    slots(vector<set<pair<Professional*, StudentGroup*>>>(data.dimensions.numSlots)) {
    NODE_COUNTER++;
};

ostream& HeurNode::print(ostream& os) const {
    os << "Node(id : " << id << ", cost : " << cost << ", slots : " << endl;
    auto cntSlot = 0;
    for (auto& slot : slots) {
        os << " " << cntSlot << " : ";
        for (auto& slotCnt : slot) {
            os << "(" << slotCnt.first << ", " << slotCnt.second << ") ";
        }
        os << endl;
        cntSlot++;
    }
    os << ")";
    return os;
};

HeurNode* firstFit(Data& data, HeurNode* node) {

}

HeurNode* firstFit(Data& data) {
    auto* firstNode = new HeurNode(data);
    cout << "firstNode " << *firstNode << endl;
    return firstNode;
}

