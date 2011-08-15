#include "DGraph.h"
#include "Node.h"
#include "Arc.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#include <queue>
#include <list>
#include <sys/stat.h>
using namespace std;

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define INFINITY INT_MAX

DGraph::DGraph() : nodeIndex(1), numNodes(0), arcIndex(0), numArcs(0), freeNode(0), source(0), sink(0), level(0), numDisabled(0) { }

// Add a node -- specific id
Node* DGraph::addNode(int id) {
  Node* n = new Node(id);
  if (freeNode <= id) {
    freeNode = id + 1;
  }
  nodes[id] = n;
  ++nodeIndex;
  return n;
}

// Create an arc with capacity cap
Arc* DGraph::addArc(Node* t, Node* h, int cap) {
  Arc* a = new Arc(t, h, cap, 0);
  arcs[arcIndex++] = a;
  t->out.push_back(a);
  h->in.push_back(a);
  return a;
}

// Read a graph from file fname in DIMACS format.
// A DIMACS file has the following format:
// - comment lines
// - problem line
// - node descriptors
// - arc descriptors
//
// 1. Comment Lines are of the form:
//    c This is a comment
//    and may appear anywhere in the file.
//
// 2. The Problem Line is of the form:
//    p max NODES ARCS
//    where NODES is the number of nodes n in the network,
//    and ARCS is the number of arcs m in the network.
//    Note that there is only one problem line per file.
//
// 3. Node Descriptors are of the form:
//    n ID WHICH
//    where ID is the node id and WHICH is s for the source and t for the sink.
//    Two node descriptors, one for the source and one for the sink, must
//    appear between the problem line and the arc descriptor lines.
//
// 4. Arc Descriptors are of the form:
//    a SRC DST CAP
//    For a directed arc (v,w) the SRC field is the node id of w and
//    DST is the node id of DST. All id numbers are integers between 1 and n.
//    CAP is the arc capacity
void DGraph::readDimacs(string fname) {
  struct stat buffer;

  // unless fname exists, exit
  if (-1 == stat(fname.c_str(), &buffer)) {
    cout << "Error: Failed to open " << fname << " -- aborting" << endl;
    exit(1);
  }
  FILE* file = fopen(fname.c_str(), "r");
  char char_in = '\0';
  int src=0, dst=0, cap=0, id=0;
  while ((char_in = getc(file)) != EOF) {
    switch (char_in) {
      case 'c':
        // it's a comment so skip to the end of the file
        while((char_in = getc(file)) != '\n');
        break;
      case 'p':
        // p max NODES ARCS
        fscanf(file, " max %d %d\n", &numNodes, &numArcs);
        nodes = (Node**)calloc(numNodes+1, sizeof(Node*));      
        arcs = (Arc**)calloc(numArcs, sizeof(Arc*));
        break;
      case 'n':
        // n ID [s,t]
        fscanf(file, " %d %c\n", &id, &char_in);
        nodes[id] = addNode(id);
        if ('s' == char_in) {
          source = nodes[id];
        }
        else if ('t' == char_in) {
          sink = nodes[id];
        }
        break;
      case 'a':
        // a SRC DST CAP
        fscanf(file, " %d %d %d\n", &src, &dst, &cap);

        // first check if the nodes exist already
        if (NULL == nodes[src]) {
          nodes[src] = addNode(src);
        }
        if (NULL == nodes[dst]) {
          nodes[dst] = addNode(dst);
        }

        // finally add the arc
        if (nodes[src]!=sink && nodes[dst]!=source) {
          addArc(nodes[src], nodes[dst], cap);
        }
        break;
    }
  }
  fclose(file);
}

// Write this graph to file fname in DIMACS format.
void DGraph::writeDimacs(string fname) {
  if (!source || !sink) return;
  FILE* file = fopen(fname.c_str(), "w");
  fprintf(file, "p max %d %d\nn %d %c\nn %d %c\n", numNodes, numArcs, source->id, 's', sink->id, 't');
  for (unsigned int i=0; i<numArcs; ++i) {
    if (arcs[i]!=NULL) {
      fprintf(file, "a %d %d %d\n", arcs[i]->tail->id, arcs[i]->head->id, arcs[i]->cap);
    }
  }
  fclose(file);
}

// Write the flow to file fname in DIMACS format.
// The graph will represent the solution to the maxflow problem, and
// the flow on each arc will be represented as: f t h v,
// where f indicates that this line contains the flow for an arc,
// t is the tail of the arc,
// h is the head of the arc, and
// v is the value of the flow
// NOTE: only arcs with non-zero flow will be displayed
void DGraph::writeFlow(string fname) {
  if (!source || !sink) return;
  FILE* file = fopen(fname.c_str(), "w");
  fprintf(file, "p max %d %d\nn %d %c\nn %d %c\n", numNodes, numArcs, source->id, 's', sink->id, 't');
  for (unsigned int i=0; i<numArcs; ++i) {
    if (arcs[i]!=NULL && arcs[i]->flow > 0) {
      fprintf(file, "f %d %d %d\n", arcs[i]->tail->id, arcs[i]->head->id, arcs[i]->flow);
    }
  }
  fclose(file);
}

// Returns the total flow on the network.
// This should be called AFTER the preflow-push algorithm has terminated,
// otherwise it getFlow might return a preflow.
int DGraph::getFlow() {
  return sink->excess;
}

void DGraph::displayFlow() {
  if (!source || !sink) return;
  cout << "--> FLOW <--" << endl;
  for (unsigned int i=0; i<numArcs; ++i) {
    if (arcs[i]!=NULL && arcs[i]->flow > 0) {
      cout << "f " << arcs[i]->tail->id << " " << arcs[i]->head->id << " " << arcs[i]->flow << endl;
    }
  }
  cout << "--> END FLOW <--" << endl << endl;
}

// An arc (i,j) in the residual network is admissible if it satisfies the
// condition that d(i) = d(j)+1; all other arcs are inadmissible.
Arc* DGraph::getAdmissibleArc(Node* i, bool& backward) {
  backward = false;
  for (vector<Arc*>::iterator it=i->out.begin(); it!=i->out.end(); ++it) {
    if ((*it)->tail->distance == (*it)->head->distance + 1 &&
        (*it)->cap - (*it)->flow > 0) {
      return *it;
    }
  }
  for (vector<Arc*>::iterator it=i->in.begin(); it!=i->in.end(); ++it) {
    if ((*it)->head->distance == (*it)->tail->distance + 1 &&
        (*it)->flow > 0) {
      backward = true;
      return *it;
    }
  }
  return NULL;
}

// =======================
// HIGHEST LABEL ALGORITHM
// =======================

// Some basic initialization stuff in preparation for the preflow-push algo
// 1. x := 0
// 2. compute the exact distance labels d(i)
// 3. x_sj = cap_sj for each arc (s,j) in A(s)
// 4. d(s) := n
//    Since we have saturated all the arcs incident to node s, none of these
//    arcs is admissible and setting d(s)=n will satisfy the validity condition
//    that d(i)<=d(j)+1 for every arc (i,j) in the residual network. Also,
//    since d(s)=n, the residual network contains no directed path from node s
//    to node t. Since distance labels are nondecreasing, we also guarantee that
//    in subsequent iterations, the residual network will never contain a
//    directed path from node s to node t, and so we will never need to push
//    flow from node s again.
void DGraph::HL_preprocess() {

  // initialize distance vector and active node list
  DLIST = vector< list<Node*> >(numNodes+1);
  activeNodes = vector< list<Node*> >(2*numNodes);

  // compute exact distance labels from sink to every other node via reverse BFS
  queue<Node*> Q;
  Q.push(sink);
//  vector<bool> visited(numNodes+1, false); // keep track of the visited nodes
  bool* visited = (bool*)calloc(numNodes+1, sizeof(bool));
  visited[sink->id] = true;
  Node* curNode;
  Node* tail;
  do {
    curNode = Q.front();
    Q.pop();
    
    // update DLIST
    if (curNode != source) {
      DLIST[curNode->distance].push_back(curNode);
    }

    for (vector<Arc*>::iterator it=curNode->in.begin(); it!=curNode->in.end(); ++it) {
      tail = (*it)->tail;
      if (!visited[tail->id]) {
        tail->distance = curNode->distance + 1;
        Q.push(tail);
        visited[tail->id] = true;
      }
    }
  } while (!Q.empty());

  // saturate all out-arcs from the source node
  for (vector<Arc*>::iterator it=source->out.begin(); it!=source->out.end(); ++it) {
    (*it)->flow = (*it)->cap;
    (*it)->head->excess = (*it)->cap;

    // update the active nodes
    // NOTE: source and sink are never active
    if (source != (*it)->head && sink != (*it)->head) {
      activeNodes[(*it)->head->distance].push_back((*it)->head);
      if ((*it)->head->distance > level) {
        level = (*it)->head->distance;
      }
    }
  }

  // set d(s) = n
  source->distance = numNodes;

  // not sure if this is necessary
//  DLIST[source->distance].push_back(source);
}

// returns true if a relabel was performed
bool DGraph::HL_push_relabel(Node* i) {

  bool relabel = false;
  bool backward = false;
  Arc* a = getAdmissibleArc(i,backward);
  if (a != NULL) {
    
    // update the active nodes
    Node* j = (backward ? a->tail : a->head);
    if (j->excess == 0 && source != j && sink != j) {
      activeNodes[j->distance].push_back(j);
    }

    // push gamma := min{ excess(i), r_ij } units of flow from node i to node j
    unsigned int r_ij = (backward ? (a->flow) : (a->cap) - (a->flow));
    unsigned int gamma = MIN(i->excess,r_ij);
    gamma = (backward ? -gamma : gamma);
    a->flow += gamma;
    a->tail->excess -= gamma;
    a->head->excess += gamma;
  }
  else {

    // replace d(i) by min{ d(j)+1 : (i,j) in A(i) and r_ij > 0 }
    unsigned int minDist = (int)INFINITY;
    for (vector<Arc*>::iterator it=i->out.begin(); it!=i->out.end(); ++it) {
      if ((*it)->cap - (*it)->flow > 0 && (*it)->head->distance + 1 < minDist) {
        // forward arc
        minDist = (*it)->head->distance + 1;
      }
    }
    for (vector<Arc*>::iterator it=i->in.begin(); it!=i->in.end(); ++it) {
      if ((*it)->flow > 0 && (*it)->tail->distance + 1 < minDist) {
        // backward arc
        minDist = (*it)->tail->distance + 1;
      }
    }

    unsigned int oldDist = i->distance;

    // node i MUST have at least one incident arc, so minDist shouldn't
    // be equal to INFINITY at this point
    i->distance = minDist;


    // remember to update the level if the distance increases
    if (i->distance > oldDist) {
      level = minDist;
    }

    // update DLIST
    DLIST.at(oldDist).remove(i);
    if (i->distance < numNodes) {
      DLIST.at(i->distance).push_back(i);
    }
    else {
      if (i->distance != numNodes) {
        i->distance = numNodes;
      }
    }

    // if DLIST(k) is empty, then all nodes in DLIST(k+1), DLIST(k+2), ...
    // are disconnected from the sink, so we can disable them
    if (DLIST.at(oldDist).empty()) {

      for (uint i=oldDist+1; i<DLIST.size(); ++i) {
        for (list<Node*>::iterator it=DLIST.at(i).begin(); it!=DLIST.at(i).end(); ++it) {
          if ((*it)->distance != numNodes) {
            (*it)->distance = numNodes;
          }
        }
      }
      level = oldDist - 1;
    }
  relabel = true;
  }
  return relabel;
}

// This is the highest-label variant of the preflow-push algorithm
void DGraph::HL_preflow_push() {
  Node* i = getActiveNode();
  while (NULL != i) {
    /*
    if (i->distance >= numNodes - numDisabled - 1) {
      // this node is disconnected from the sink, so we disable it
      i->enabled = false;
      numDisabled++;
      i = getActiveNode();
      continue;
    }
    */
    while (i->excess > 0) {
      if (HL_push_relabel(i)) {
        // node was relabelled so it is still active
        activeNodes[i->distance].push_back(i);
        break;
      }
    }
    i = getActiveNode();
  }
}

Node* DGraph::getActiveNode() {
  Node* n;
  for (uint i=level; i>0; --i) {
    if (!activeNodes[i].empty()) {
      n = activeNodes[i].front();
      activeNodes[i].pop_front();
      if (n->distance != numNodes) {
        return n;
      }
      else {
        return getActiveNode();
      }
    }
  }
  return NULL;
}

// HL_preflow_push terminates with a preflow.
// It may not be a lflow since some excess may reside at disabled nodes.
// Thus, we need to convert the maximum preflow into a maximum flow.
void DGraph::HL_postprocess() {

  // initialize distance vector and active node list
  DLIST = vector< list<Node*> >(numNodes+1);
  activeNodes = vector< list<Node*> >(2*numNodes);

  // reset level
  level = 0;

  // compute exact distance labels from source to every other node via BFS
  queue<Node*> Q;
  Q.push(source);
//  vector<bool> visited(numNodes+1, false); // keep track of the visited nodes
  bool* visited = (bool*)calloc(numNodes+1, sizeof(bool));
  visited[source->id] = true;
  source->distance = 0;
  Node* curNode;
  Node* head;
  do {
    curNode = Q.front();
    Q.pop();
    
    // update DLIST
    DLIST[curNode->distance].push_back(curNode);

    // update active nodes
    if (curNode != source && curNode != sink) {
      activeNodes.at(curNode->distance).push_back(curNode);
      if (curNode->distance > level) {
        level = curNode->distance;
      }
    }

    for (vector<Arc*>::iterator it=curNode->out.begin(); it!=curNode->out.end(); ++it) {
      head = (*it)->head;
      if (!visited[head->id]) {
        head->distance = curNode->distance + 1;
        Q.push(head);
        visited[head->id] = true;
      }
    }
  } while (!Q.empty());

  sink->distance = numNodes;

  // push all the excess back to the source
  HL_preflow_push();
}

void DGraph::preflow_push() {
  HL_preprocess();
  HL_preflow_push();
  HL_postprocess();
}

void DGraph::checkFlow() {
  for (unsigned int i=1; i<numNodes; ++i) {
    if (nodes[i]!=source && nodes[i]!=sink && nodes[i]->getExcess() != 0) {
      cout << "WTF node: " << nodes[i]->id << " is active " << endl;
    }
  }
}
