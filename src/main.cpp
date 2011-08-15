#include "DGraph.h"
#include "Node.h"
#include "Arc.h"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[]) {
  string fname = "challenge.net";
  string fout = "result.net";
  if (1 == argc) {
    cout << "Warning: No input file specified -- using challenge.net" << endl;
  }
  else {
    // use file provided in argument
    fname = argv[1];
  }
  DGraph d;

  // read in graph from fname
  d.readDimacs(fname);

  // run preflow/push algorithm
  d.preflow_push();

  // print the value of the flow
  cout << "Flow: " << d.getFlow() << endl;

  // force exit
  exit(0);
}
