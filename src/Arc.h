#ifndef __ARC_H
#define __ARC_H

class Node;

class Arc {
  public:
    Arc() {};
    ~Arc() {};
    Arc(Node* t, Node* h);
    Arc(Node* t, Node* h, int ca, int co);

    // TODO: make this stuff private
    Node* tail;
    Node* head;
    unsigned int cap; // arc capacity
    unsigned int flow; // flow on arc
};

#endif
