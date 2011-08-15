#include "Node.h"
#include "Arc.h"

Node::Node(int i) : id(i), distance(0), excess(0) {}

int Node::getExcess() {
  int total_in = 0;
  int total_out = 0;
   for (std::vector<Arc*>::iterator it=in.begin(); it!=in.end(); ++it) {
    total_in += (*it)->flow;
  } 
  for (std::vector<Arc*>::iterator it=out.begin(); it!=out.end(); ++it) {
    total_out += (*it)->flow;
  }
  return total_in - total_out;
}
