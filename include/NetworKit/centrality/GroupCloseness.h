/*
 * GroupCloseness.h
 *
 *  Created on: 03.10.2016
 *      Author: elisabetta bergamini
 */

#ifndef GROUPCLOSENESS_H_
#define GROUPCLOSENESS_H_
#include "../graph/Graph.h"
#include "../base/Algorithm.h"

namespace NetworKit {

/**
 * @ingroup centrality
 */
class GroupCloseness : public Algorithm {
public:
	/**
	 * Finds the group of nodes with highest (group) closeness centrality.
	 * The algorithm is the one proposed in Bergamini et al., ALENEX 2018 and
	 * finds a solution that is a (1-1/e)-approximation of the optimum.
	 * The worst-case running time of this approach is quadratic, but usually
	 * much faster in practice.
	 *
	 * @param G An unweighted graph.
	 * @param k Size of the group of nodes
	 * @param H If equal 0, simply runs the algorithm proposed in Bergamini et al.. If > 0, interrupts all BFSs after H iterations (suggested for very large networks).
	 * @
	 */
	GroupCloseness(const Graph& G, count k = 1, count H=0);

	/**
	* Computes the group with maximum closeness on the graph passed in the constructor.
	*/
	void run();

	/**
	* Returns group with maximum closeness.
	*/
	std::vector<node> groupMaxCloseness();

	/**
	* Computes farness (i.e., inverse of the closeness) for a given group (stopping after H iterations if H > 0).
	*/
	double computeFarness(std::vector<node> S, count H = std::numeric_limits<count>::max());


protected:
	edgeweight computeImprovement(node u, count n, Graph& G, count h);
	std::vector<count> newDistances(node u, count n, Graph& G, count h);
	Graph G;
	count k = 1;
	std::vector<count> D;
	count iters;
	count maxD;
	std::vector<count> d;
	std::vector<count> d1;
	std::vector<node> S;
	count H = 0;
};



inline std::vector<node> GroupCloseness::groupMaxCloseness() {
	if (!hasRun) throw std::runtime_error("Call run method first");
	return S;
}

} /* namespace NetworKit */
#endif /* GROUPCLOSENESS_H_ */