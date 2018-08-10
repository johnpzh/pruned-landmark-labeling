#include <cstdlib>
#include <iostream>
#include "pruned_landmark_labeling.h"

using namespace std;

int main(int argc, char **argv) {
  if (argc != 3) {
    cerr << "usage: construct_index GRAPH INDEX" << endl;
    exit(EXIT_FAILURE);
  }

  PrunedLandmarkLabeling<50> pll;
  // By Johnpzh
  char *filename = argv[1];
  EdgeListReader reader(
		  '\t',
		  0,
		  "#",
		  true,
		  false);
  Graph G = reader.read(string(filename));
  if (!pll.ConstructIndex(G)) {
    cerr << "error: Load or construction failed" << endl;
    exit(EXIT_FAILURE);
  }

  //if (!pll.ConstructIndex(argv[1])) {
  //  cerr << "error: Load or construction failed" << endl;
  //  exit(EXIT_FAILURE);
  //}
  // End By Johnpzh
  pll.PrintStatistics();

  if (!pll.StoreIndex(argv[2])) {
    cerr << "error: Store failed" << endl;
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
