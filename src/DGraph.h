#ifndef __DGRAPH_H
#define __DGRAPH_H

#include <string>
#include <vector>
#include <list>
#include <queue>
#include <map>

#include "Node.h"
#include "Arc.h"

struct nodeCmp {
  bool operator() (const Node* na, const Node* nb) {
    return (na->distance > nb->distance);
  }
};

class DGraph {
  public:
    DGraph();
    ~DGraph() {};
    Node* addNode(int id);
    Arc* addArc(Node* t, Node* h, int cap);
    void readDimacs(std::string fname);
    void writeDimacs(std::string fname);
    void writeFlow(std::string fname);
    int getFlow();

    void HL_preprocess();
    bool HL_push_relabel(Node* i);
    void HL_preflow_push();
    void HL_postprocess();
    void preflow_push();
    Node* getActiveNode();
    Arc* getAdmissibleArc(Node* i, bool& backward);

    void HL_relabel(Node* i);
    void HL_discharge(Node* i);

    void displayFlow();
    void checkFlow();

    Node** nodes;
    int nodeIndex;
    unsigned int numNodes;
    Arc** arcs;
    int arcIndex;
    unsigned int numArcs;
    int freeNode; // unused node index
    Node* source;
    Node* sink;
    std::vector< std::list<Node*> > activeNodes;
    std::vector< std::list<Node*> > DLIST; // DLIST(i) contains node at distance i from the sink
    unsigned int level;
    int numDisabled;
};

#endif
