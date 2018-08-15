#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include "pruned_landmark_labeling.h"

using namespace std;

//template<typename T>
//void labeling(
//			const T &pll, 
//			const char *filename,
//			const char *out_filename,
//			const char &separator,
//			const uint64_t &start_id,
//			const bool &is_directed)
//{
//  EdgeListReader reader(
//		  separator,
//		  start_id,
//		  "#",
//		  true,
//		  is_directed);
//  Graph G = reader.read(string(filename));
//  if (!pll.ConstructIndex(G)) {
//    cerr << "error: Load or construction failed" << endl;
//    exit(EXIT_FAILURE);
//  }
//  //if (!pll.ConstructIndex(argv[1])) {
//  //  cerr << "error: Load or construction failed" << endl;
//  //  exit(EXIT_FAILURE);
//  //}
//  // End By Johnpzh
//  pll.PrintStatistics();
//
//  if (!pll.StoreIndex(out_filename)) {
//    cerr << "error: Store failed" << endl;
//    exit(EXIT_FAILURE);
//  }
//}

int main(int argc, char **argv) {
  // By Johnpzh
	char separator = ' ';
	//uint64_t kNum = 50;
	bool is_directed = false;
	uint64_t start_id = 1;
	int opt;
	if (argc < 4) {
		//fprintf(stderr, "Usage: ./construct_index <input_data> <output_index> [-s | -t] [-k 50|500|1000|2000] [-d] [-i StartID]\n");
		fprintf(stderr, "Usage: ./construct_index <input_data> <output_index> [-s | -t] [-d] [-i StartID]\n");
		exit(EXIT_FAILURE);
	}
	while ((opt = getopt(argc, argv, "stk:di:")) != -1) {
		switch (opt) {
			case 't':
				separator = '\t';
				break;
			//case 'k':
			//	kNum = strtoul(optarg, NULL, 0);
			//	if (50 == kNum) {
			//		PrunedLandmarkLabeling<50> pll;
			//		labeling(
			//				pll, 
			//				argv[1],
			//				argv[2],
			//				separator,
			//				start_id,
			//				is_directed);
			//	} else if (500 == kNum) {
			//		PrunedLandmarkLabeling<500> pll;
			//		labeling(
			//				pll, 
			//				argv[1],
			//				argv[2],
			//				separator,
			//				start_id,
			//				is_directed);
			//	} else if (1000 == kNum) {
			//		PrunedLandmarkLabeling<1000> pll;
			//		labeling(
			//				pll, 
			//				argv[1],
			//				argv[2],
			//				separator,
			//				start_id,
			//				is_directed);
			//	} else if (2000 == kNum) {
			//		PrunedLandmarkLabeling<2000> pll;
			//		labeling(
			//				pll, 
			//				argv[1],
			//				argv[2],
			//				separator,
			//				start_id,
			//				is_directed);
			//	} else {
			//		fprintf(stderr, "Error: unknown number for -k (50|500|1000|2000)\n");
			//		exit(EXIT_FAILURE);
			//	}
			//	break;
			case 'd':
				is_directed = true;
				break;
			case 'i':
				start_id = strtoul(optarg, NULL, 0);
				break;
			default:
				fprintf(stderr, "Error: unknown opt %c.\n", opt);
				exit(EXIT_FAILURE);
		}
	}
  //if (argc != 3) {
  //  cerr << "usage: construct_index GRAPH INDEX" << endl;
  //  exit(EXIT_FAILURE);
  //}
  PrunedLandmarkLabeling<50> pll;

  char *filename = argv[1];
  EdgeListReader reader(
    	  separator,
    	  start_id,
    	  "#",
    	  true,
    	  is_directed);
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
