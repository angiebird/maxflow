#ifndef __NODE_H
#define __NODE_H

#include <vector>

class Arc;

class Node {
  public:
    Node() {};
    ~Node() {};
    Node(int i);
    int getExcess();

    // TODO: make this stuff private
    int id;
    std::vector<Arc*> in; // list of in-arcs
    std::vector<Arc*> out; // list of out-arcs
    std::vector<Arc*>::iterator curInArc;
    std::vector<Arc*>::iterator curOutArc;

    unsigned int distance;
    unsigned int excess;
};

#endif
