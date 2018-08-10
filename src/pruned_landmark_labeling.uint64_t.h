// Copyright 2013, Takuya Akiba
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Takuya Akiba nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PRUNED_LANDMARK_LABELING_H_
#define PRUNED_LANDMARK_LABELING_H_

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdint.h>
#include <sys/time.h>
#include <climits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <algorithm>
#include <fstream>
#include <utility>
#include <xmmintrin.h>

// By Johnpzh
#include "profiler.h"
#include "NetworKit/graph/Graph.h"
#include "NetworKit/io/EdgeListReader.h"
#include "NetworKit/centrality/ApproxBetweenness.h"
using NetworKit::Graph;
using NetworKit::EdgeListReader;
using NetworKit::ApproxBetweenness;
using NetworKit::node;
// End By Johnpzh

//
// NOTE: Currently only unweighted and undirected graphs are supported.
//
template<uint64_t kNumBitParallelRoots = 500>
class PrunedLandmarkLabeling {
 public:
  // Constructs an index from a graph, given as a list of edges.
  // Vertices should be described by numbers starting from zero.
  // Returns |true| when successful.
  // By Johnpzh
  bool ConstructIndex(const std::vector<std::pair<uint64_t, uint64_t> > &es);
  //bool ConstructIndex(const std::vector<std::pair<uint64_t, uint64_t> > &es);
  // End By Johnpzh
  bool ConstructIndex(std::istream &ifs);
  bool ConstructIndex(const char *filename);

  // By Johnpzh
  bool ConstructIndex(const Graph &G);
  // End By Johnpzh

  // Returns distance vetween vertices |v| and |w| if they are connected.
  // Otherwise, returns |INT_MAX|.
  // By Johnpzh
  inline uint64_t QueryDistance(uint64_t v, uint64_t w);
  //inline uint64_t QueryDistance(uint64_t v, uint64_t w);
  // End By Johnpzh

  // Loads an index. Returns |true| when successful.
  bool LoadIndex(std::istream &ifs);
  bool LoadIndex(const char *filename);

  // Stores the index. Returns |true| when successful.
  bool StoreIndex(std::ostream &ofs);
  bool StoreIndex(const char *filename);

  uint64_t GetNumVertices() { return num_v_; }
  void Free();
  void PrintStatistics();

  PrunedLandmarkLabeling()
      : num_v_(0), index_(NULL), time_load_(0), time_indexing_(0) {}
  virtual ~PrunedLandmarkLabeling() {
    Free();
  }

 private:
  static const uint8_t INF8;  // For unreachable pairs

  struct index_t {
    uint8_t bpspt_d[kNumBitParallelRoots]; // ??
    uint64_t bpspt_s[kNumBitParallelRoots][2];  // [0]: S^{-1}, [1]: S^{0}
    uint64_t *spt_v; // vertex ID
    uint8_t *spt_d; // distance
  } __attribute__((aligned(64)));  // Aligned for cache lines

  // By Johnpzh
  uint64_t num_v_;
  //uint64_t num_v_;
  // End By Johnpzh
  index_t *index_;

  double GetCurrentTimeSec() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
  }

  // Statistics
  double time_load_, time_indexing_;
};

template<uint64_t kNumBitParallelRoots>
const uint8_t PrunedLandmarkLabeling<kNumBitParallelRoots>::INF8 = 100;

template<uint64_t kNumBitParallelRoots>
bool PrunedLandmarkLabeling<kNumBitParallelRoots>
::ConstructIndex(const char *filename) {
  std::ifstream ifs(filename);
  return ifs && ConstructIndex(ifs);
}

template<uint64_t kNumBitParallelRoots>
bool PrunedLandmarkLabeling<kNumBitParallelRoots>
::ConstructIndex(std::istream &ifs) {
  std::vector<std::pair<uint64_t, uint64_t> > es;
  for (uint64_t v, w; ifs >> v >> w; ) {
    es.push_back(std::make_pair(v, w));
  }
  if (ifs.bad()) return false;
  ConstructIndex(es);
  return true;
}

template<uint64_t kNumBitParallelRoots>
bool PrunedLandmarkLabeling<kNumBitParallelRoots>
// By Johnpzh
::ConstructIndex(const std::vector<std::pair<uint64_t, uint64_t> > &es) 
//::ConstructIndex(const std::vector<std::pair<uint64_t, uint64_t> > &es) 
// End By Johnpzh
{
	Profiler profiler;
  //
  // Prepare the adjacency list and index space
  //
  Free();
  time_load_ = -GetCurrentTimeSec();
  
  // By Johnpzh
  uint64_t E = es.size();
  uint64_t &V = num_v_;
  //uint64_t &V = num_v_;
  // End By Johnpzh
  V = 0;
  for (size_t i = 0; i < es.size(); ++i) {
    V = std::max(V, std::max(es[i].first, es[i].second) + 1);
  }

//  profiler.print("Vertices: ", V, " Edges: ", E, "\n");

  std::vector<std::vector<uint64_t> > adj(V);
  for (size_t i = 0; i < es.size(); ++i) {
    uint64_t v = es[i].first, w = es[i].second;
    adj[v].push_back(w);
    adj[w].push_back(v);
  }
  time_load_ += GetCurrentTimeSec();

  index_ = (index_t*)memalign(64, V * sizeof(index_t));
  if (index_ == NULL) {
    num_v_ = 0;
    return false;
  }
  for (uint64_t v = 0; v < V; ++v) {
    index_[v].spt_v = NULL;
    index_[v].spt_d = NULL;
  }
//  profiler.print("Input finished.\n");

  //
  // Order vertices by decreasing order of degree
  //
  time_indexing_ = -GetCurrentTimeSec();
  std::vector<uint64_t> inv(V);  // new label -> old label
  {
    // Order
    std::vector<std::pair<float, uint64_t> > deg(V);
    for (uint64_t v = 0; v < V; ++v) {
      // We add a random value here to diffuse nearby vertices
      deg[v] = std::make_pair(adj[v].size() + float(rand()) / RAND_MAX, v);
    }
    std::sort(deg.rbegin(), deg.rend());
    for (uint64_t i = 0; i < V; ++i) inv[i] = deg[i].second;

    // Relabel the vertex IDs
    std::vector<uint64_t> rank(V);
    for (uint64_t i = 0; i < V; ++i) rank[deg[i].second] = i; // rank[old_label] = new_label
    std::vector<std::vector<uint64_t> > new_adj(V);
    for (uint64_t v = 0; v < V; ++v) {
      for (size_t i = 0; i < adj[v].size(); ++i) {
        new_adj[rank[v]].push_back(rank[adj[v][i]]);
      }
    }
    adj.swap(new_adj);
  } // Get the adj as the new_adj now.
//  profiler.print("Order finished.\n");
  profiler.reset();
  //
  // Bit-parallel labeling
  //
  std::vector<bool> usd(V, false);  // Used as root? (in new label)
  {
    std::vector<uint8_t> tmp_d(V);
    std::vector<std::pair<uint64_t, uint64_t> > tmp_s(V);
    std::vector<uint64_t> que(V);
    std::vector<std::pair<uint64_t, uint64_t> > sibling_es(E); // E_0
    std::vector<std::pair<uint64_t, uint64_t> > child_es(E); // E_1

    uint64_t r = 0;
    for (uint64_t i_bpspt = 0; i_bpspt < kNumBitParallelRoots; ++i_bpspt) {
      while (r < V && usd[r]) ++r;
      if (r == V) {
        for (uint64_t v = 0; v < V; ++v) index_[v].bpspt_d[i_bpspt] = INF8;
        continue;
      }
      usd[r] = true;
      profiler.bfs_click();

      fill(tmp_d.begin(), tmp_d.end(), INF8);
      fill(tmp_s.begin(), tmp_s.end(), std::make_pair(0, 0));

      uint64_t que_t0 = 0;
      uint64_t que_t1 = 0; // last que.size
      uint64_t que_h = 0; // que.size
      que[que_h++] = r;
      tmp_d[r] = 0;
      que_t1 = que_h;

      uint64_t ns = 0;
      std::vector<uint64_t> vs;
      sort(adj[r].begin(), adj[r].end());
      for (size_t i = 0; i < adj[r].size(); ++i) {
        uint64_t v = adj[r][i];
        if (!usd[v]) {
          usd[v] = true; // ?? why usd[v] = true?
          profiler.bfs_click();
          que[que_h++] = v;
          tmp_d[v] = 1;
          tmp_s[v].first = 1ULL << ns; // ?? why 1ULL << ns
          vs.push_back(v);
          if (++ns == 64) break;
        }
      } // line 4

      for (uint64_t d = 0; que_t0 < que_h; ++d) {
        uint64_t num_sibling_es = 0, num_child_es = 0;

        for (uint64_t que_i = que_t0; que_i < que_t1; ++que_i) {
          uint64_t v = que[que_i];

          for (size_t i = 0; i < adj[v].size(); ++i) {
            uint64_t tv = adj[v][i];
            uint64_t td = d + 1;

            if (d > tmp_d[tv]) {
            	;
            } else if (d == tmp_d[tv]) {
              if (v < tv) { // ?? Why need v < tv ?
                sibling_es[num_sibling_es].first  = v;
                sibling_es[num_sibling_es].second = tv; // line 19: E_0 union (v, tv)
                ++num_sibling_es;
              }
            } else { // d < tmp_d[tv]
              if (tmp_d[tv] == INF8) {
                que[que_h++] = tv;
                tmp_d[tv] = td;
              }
              child_es[num_child_es].first  = v;
              child_es[num_child_es].second = tv; // line 14: E_1 union (v, tv)
              ++num_child_es;
            }
          }
        }

        for (uint64_t i = 0; i < num_sibling_es; ++i) {
          uint64_t v = sibling_es[i].first, w = sibling_es[i].second;
          tmp_s[v].second |= tmp_s[w].first; // ?? Why need this?
          tmp_s[w].second |= tmp_s[v].first; // Line 21: S^{0}[w] |= S^{-1}[v]
        }
        for (uint64_t i = 0; i < num_child_es; ++i) {
          uint64_t v = child_es[i].first, c = child_es[i].second;
          tmp_s[c].first  |= tmp_s[v].first; // Line 23: S^{-1}[c] |= S^{-1}[v]
          tmp_s[c].second |= tmp_s[v].second; // Line 24: S^{0}[c] |= S^{0}[v]
        }

//        profiler.add_label(num_sibling_es + num_child_es);
//        if (profiler.get_label_count() % 1E6 == 0) {
//      	  profiler.print("Label: ", profiler.get_label_count(), " Time: ", profiler.time_click(), "\n");
//        }
        que_t0 = que_t1;
        que_t1 = que_h;

//        profiler.print("d: ", d, " que_t0: ", que_t0, " que_h: ", que_h, "\n");
        if (profiler.equal_values(d, uint64_t(INF8))) {
        	profiler.print("d: ", d, " que_t0: ", que_t0, " que_h: ", que_h, "\n");
        	if (que_t0 >= que_h) {
        		exit(2);
        	}
        }

      }

      for (uint64_t v = 0; v < V; ++v) {
        index_[inv[v]].bpspt_d[i_bpspt] = tmp_d[v];
        index_[inv[v]].bpspt_s[i_bpspt][0] = tmp_s[v].first;
        index_[inv[v]].bpspt_s[i_bpspt][1] = tmp_s[v].second & ~tmp_s[v].first;
      }

//      profiler.print("The ", profiler.get_bfs_count(), "  BFS finished.\n");
    }
  }
//  profiler.print("Bit parallel finished.\n");

  //
  // Pruned labeling
  //
  {
    // Sentinel (V, INF8) is added to all the vertices
    std::vector<std::pair<std::vector<uint64_t>, std::vector<uint8_t> > >
        tmp_idx(V, make_pair(std::vector<uint64_t>(1, V),
                             std::vector<uint8_t>(1, INF8)));

    std::vector<bool> vis(V);
    std::vector<uint64_t> que(V);
    std::vector<uint8_t> dst_r(V + 1, INF8);

    for (uint64_t r = 0; r < V; ++r) {
      if (usd[r]) continue;
      index_t &idx_r = index_[inv[r]];
      const std::pair<std::vector<uint64_t>, std::vector<uint8_t> >
          &tmp_idx_r = tmp_idx[r];
      for (size_t i = 0; i < tmp_idx_r.first.size(); ++i) {
        dst_r[tmp_idx_r.first[i]] = tmp_idx_r.second[i];
      }

      uint64_t que_t0 = 0, que_t1 = 0, que_h = 0;
      que[que_h++] = r;
      vis[r] = true;
      que_t1 = que_h;

      for (uint64_t d = 0; que_t0 < que_h; ++d) {
        for (uint64_t que_i = que_t0; que_i < que_t1; ++que_i) {
          uint64_t v = que[que_i];
          std::pair<std::vector<uint64_t>, std::vector<uint8_t> >
              &tmp_idx_v = tmp_idx[v];
          index_t &idx_v = index_[inv[v]];

          // Prefetch
          _mm_prefetch(&idx_v.bpspt_d[0], _MM_HINT_T0);
          _mm_prefetch(&idx_v.bpspt_s[0][0], _MM_HINT_T0);
          _mm_prefetch(&tmp_idx_v.first[0], _MM_HINT_T0);
          _mm_prefetch(&tmp_idx_v.second[0], _MM_HINT_T0);

          // Prune?
          if (usd[v]) continue;
          for (uint64_t i = 0; i < kNumBitParallelRoots; ++i) {
            uint64_t td = idx_r.bpspt_d[i] + idx_v.bpspt_d[i];
            if (td - 2 <= d) {
              td +=
                  (idx_r.bpspt_s[i][0] & idx_v.bpspt_s[i][0]) ? -2 :
                  ((idx_r.bpspt_s[i][0] & idx_v.bpspt_s[i][1]) |
                   (idx_r.bpspt_s[i][1] & idx_v.bpspt_s[i][0]))
                  ? -1 : 0;
              if (td <= d) goto pruned;
            }
          }
          for (size_t i = 0; i < tmp_idx_v.first.size(); ++i) {
            uint64_t w = tmp_idx_v.first[i];
            uint64_t td = tmp_idx_v.second[i] + dst_r[w];
            if (td <= d) goto pruned;
          }

          // Traverse
          tmp_idx_v.first .back() = r;
          tmp_idx_v.second.back() = d;
          tmp_idx_v.first .push_back(V);
          tmp_idx_v.second.push_back(INF8);
          for (size_t i = 0; i < adj[v].size(); ++i) {
            uint64_t w = adj[v][i];
            if (!vis[w]) {
              que[que_h++] = w;
              vis[w] = true;
            }
          }

          profiler.add_label(1);
//          if (profiler.get_label_count() % 1E6 == 0) {
//        	  profiler.print("Label: ", profiler.get_label_count(), " Time: ", profiler.time_click(), "\n");
//          }

       pruned:
          {}
        }

        que_t0 = que_t1;
        que_t1 = que_h;

//        profiler.print("d: ", d, " que_t0: ", que_t0, " que_h: ", que_h, "\n");
        //if (profiler.equal_values(d, uint64_t(INF8))) {
        //	profiler.print("d: ", d, " que_t0: ", que_t0, " que_h: ", que_h, "\n");
        //	if (que_t0 >= que_h) {
        //		exit(2);
        //	}
        //}
      }

      for (uint64_t i = 0; i < que_h; ++i) vis[que[i]] = false;
      for (size_t i = 0; i < tmp_idx_r.first.size(); ++i) {
        dst_r[tmp_idx_r.first[i]] = INF8;
      }
      usd[r] = true;
//      printf("The %u BFS finished (%d vertices).\n", profiler.bfs_click(), que_h);
      profiler.print(profiler.bfs_click(), " BFS finished. ", que_h, " vertices. ", profiler.get_label_count(), " ", profiler.time_click(), "\n");
    }

    for (uint64_t v = 0; v < V; ++v) {
      uint64_t k = tmp_idx[v].first.size();
      index_[inv[v]].spt_v = (uint64_t*)memalign(64, k * sizeof(uint64_t));
      index_[inv[v]].spt_d = (uint8_t *)memalign(64, k * sizeof(uint8_t ));
      if (!index_[inv[v]].spt_v || !index_[inv[v]].spt_d) {
        Free();
        return false;
      }
      for (uint64_t i = 0; i < k; ++i) index_[inv[v]].spt_v[i] = tmp_idx[v].first[i];
      for (uint64_t i = 0; i < k; ++i) index_[inv[v]].spt_d[i] = tmp_idx[v].second[i];
      tmp_idx[v].first.clear();
      tmp_idx[v].second.clear();
    }
  }
  profiler.print("PLL finished.\n");

  time_indexing_ += GetCurrentTimeSec();
  return true;
}

// By Johnpzh
template<uint64_t kNumBitParallelRoots>
bool PrunedLandmarkLabeling<kNumBitParallelRoots>
::ConstructIndex(const Graph &G) 
//::ConstructIndex(const std::vector<std::pair<uint64_t, uint64_t> > &es) 
{
  // Generate es from G
	const std::vector<std::pair<node, node> > &es = G.edges();
	//const std::vector<std::pair<uint64_t, uint64_t> > &es = G.edges();
  // End Generate es from G
	Profiler profiler;
  //
  // Prepare the adjacency list and index space
  //
  Free();
  time_load_ = -GetCurrentTimeSec();
  uint64_t E = es.size();
  uint64_t &V = num_v_;
  //uint64_t E = es.size();
  //uint64_t &V = num_v_;
  V = 0;
  for (size_t i = 0; i < es.size(); ++i) {
    V = std::max(V, std::max(es[i].first, es[i].second) + 1);
  }

//  profiler.print("Vertices: ", V, " Edges: ", E, "\n");

  std::vector<std::vector<uint64_t> > adj(V);
  for (size_t i = 0; i < es.size(); ++i) {
    uint64_t v = es[i].first, w = es[i].second;
    adj[v].push_back(w);
    adj[w].push_back(v);
  }
  time_load_ += GetCurrentTimeSec();

  index_ = (index_t*)memalign(64, V * sizeof(index_t));
  if (index_ == NULL) {
    num_v_ = 0;
    return false;
  }
  for (uint64_t v = 0; v < V; ++v) {
    index_[v].spt_v = NULL;
    index_[v].spt_d = NULL;
  }
//  profiler.print("Input finished.\n");

  //
  // Order vertices by decreasing order of degree
  //
  time_indexing_ = -GetCurrentTimeSec();
  std::vector<uint64_t> inv(V);  // new label -> old label
  {
    // Order
    std::vector<std::pair<float, uint64_t> > deg(V);
    for (uint64_t v = 0; v < V; ++v) {
      // We add a random value here to diffuse nearby vertices
      deg[v] = std::make_pair(adj[v].size() + float(rand()) / RAND_MAX, v);
    }
    std::sort(deg.rbegin(), deg.rend());
    for (uint64_t i = 0; i < V; ++i) inv[i] = deg[i].second;

    // Relabel the vertex IDs
    std::vector<uint64_t> rank(V);
    for (uint64_t i = 0; i < V; ++i) rank[deg[i].second] = i; // rank[old_label] = new_label
    std::vector<std::vector<uint64_t> > new_adj(V);
    for (uint64_t v = 0; v < V; ++v) {
      for (size_t i = 0; i < adj[v].size(); ++i) {
        new_adj[rank[v]].push_back(rank[adj[v][i]]);
      }
    }
    adj.swap(new_adj);
  } // Get the adj as the new_adj now.
//  profiler.print("Order finished.\n");
  profiler.reset();
  //
  // Bit-parallel labeling
  //
  std::vector<bool> usd(V, false);  // Used as root? (in new label)
  {
    std::vector<uint8_t> tmp_d(V);
    std::vector<std::pair<uint64_t, uint64_t> > tmp_s(V);
    std::vector<uint64_t> que(V);
    std::vector<std::pair<uint64_t, uint64_t> > sibling_es(E); // E_0
    std::vector<std::pair<uint64_t, uint64_t> > child_es(E); // E_1

    uint64_t r = 0;
    for (uint64_t i_bpspt = 0; i_bpspt < kNumBitParallelRoots; ++i_bpspt) {
      while (r < V && usd[r]) ++r;
      if (r == V) {
        for (uint64_t v = 0; v < V; ++v) index_[v].bpspt_d[i_bpspt] = INF8;
        continue;
      }
      usd[r] = true;
      profiler.bfs_click();

      fill(tmp_d.begin(), tmp_d.end(), INF8);
      fill(tmp_s.begin(), tmp_s.end(), std::make_pair(0, 0));

      uint64_t que_t0 = 0;
      uint64_t que_t1 = 0; // last que.size
      uint64_t que_h = 0; // que.size
      que[que_h++] = r;
      tmp_d[r] = 0;
      que_t1 = que_h;

      uint64_t ns = 0;
      std::vector<uint64_t> vs;
      sort(adj[r].begin(), adj[r].end());
      for (size_t i = 0; i < adj[r].size(); ++i) {
        uint64_t v = adj[r][i];
        if (!usd[v]) {
          usd[v] = true; // ?? why usd[v] = true?
          profiler.bfs_click();
          que[que_h++] = v;
          tmp_d[v] = 1;
          tmp_s[v].first = 1ULL << ns; // ?? why 1ULL << ns
          vs.push_back(v);
          if (++ns == 64) break;
        }
      } // line 4

      for (uint64_t d = 0; que_t0 < que_h; ++d) {
        uint64_t num_sibling_es = 0, num_child_es = 0;

        for (uint64_t que_i = que_t0; que_i < que_t1; ++que_i) {
          uint64_t v = que[que_i];

          for (size_t i = 0; i < adj[v].size(); ++i) {
            uint64_t tv = adj[v][i];
            uint64_t td = d + 1;

            if (d > tmp_d[tv]) {
            	;
            } else if (d == tmp_d[tv]) {
              if (v < tv) { // ?? Why need v < tv ?
                sibling_es[num_sibling_es].first  = v;
                sibling_es[num_sibling_es].second = tv; // line 19: E_0 union (v, tv)
                ++num_sibling_es;
              }
            } else { // d < tmp_d[tv]
              if (tmp_d[tv] == INF8) {
                que[que_h++] = tv;
                tmp_d[tv] = td;
              }
              child_es[num_child_es].first  = v;
              child_es[num_child_es].second = tv; // line 14: E_1 union (v, tv)
              ++num_child_es;
            }
          }
        }

        for (uint64_t i = 0; i < num_sibling_es; ++i) {
          uint64_t v = sibling_es[i].first, w = sibling_es[i].second;
          tmp_s[v].second |= tmp_s[w].first; // ?? Why need this?
          tmp_s[w].second |= tmp_s[v].first; // Line 21: S^{0}[w] |= S^{-1}[v]
        }
        for (uint64_t i = 0; i < num_child_es; ++i) {
          uint64_t v = child_es[i].first, c = child_es[i].second;
          tmp_s[c].first  |= tmp_s[v].first; // Line 23: S^{-1}[c] |= S^{-1}[v]
          tmp_s[c].second |= tmp_s[v].second; // Line 24: S^{0}[c] |= S^{0}[v]
        }

//        profiler.add_label(num_sibling_es + num_child_es);
//        if (profiler.get_label_count() % 1E6 == 0) {
//      	  profiler.print("Label: ", profiler.get_label_count(), " Time: ", profiler.time_click(), "\n");
//        }
        que_t0 = que_t1;
        que_t1 = que_h;

//        profiler.print("d: ", d, " que_t0: ", que_t0, " que_h: ", que_h, "\n");
        if (profiler.equal_values(d, uint64_t(INF8))) {
        	profiler.print("d: ", d, " que_t0: ", que_t0, " que_h: ", que_h, "\n");
        	if (que_t0 >= que_h) {
        		exit(2);
        	}
        }

      }

      for (uint64_t v = 0; v < V; ++v) {
        index_[inv[v]].bpspt_d[i_bpspt] = tmp_d[v];
        index_[inv[v]].bpspt_s[i_bpspt][0] = tmp_s[v].first;
        index_[inv[v]].bpspt_s[i_bpspt][1] = tmp_s[v].second & ~tmp_s[v].first;
      }

//      profiler.print("The ", profiler.get_bfs_count(), "  BFS finished.\n");
    }
  }
//  profiler.print("Bit parallel finished.\n");

  //
  // Pruned labeling
  //
  {
    // Sentinel (V, INF8) is added to all the vertices
    std::vector<std::pair<std::vector<uint64_t>, std::vector<uint8_t> > >
        tmp_idx(V, make_pair(std::vector<uint64_t>(1, V),
                             std::vector<uint8_t>(1, INF8)));

    std::vector<bool> vis(V);
    std::vector<uint64_t> que(V);
    std::vector<uint8_t> dst_r(V + 1, INF8);

    for (uint64_t r = 0; r < V; ++r) {
      if (usd[r]) continue;
      index_t &idx_r = index_[inv[r]];
      const std::pair<std::vector<uint64_t>, std::vector<uint8_t> >
          &tmp_idx_r = tmp_idx[r];
      for (size_t i = 0; i < tmp_idx_r.first.size(); ++i) {
        dst_r[tmp_idx_r.first[i]] = tmp_idx_r.second[i];
      }

      uint64_t que_t0 = 0, que_t1 = 0, que_h = 0;
      que[que_h++] = r;
      vis[r] = true;
      que_t1 = que_h;

      for (uint64_t d = 0; que_t0 < que_h; ++d) {
        for (uint64_t que_i = que_t0; que_i < que_t1; ++que_i) {
          uint64_t v = que[que_i];
          std::pair<std::vector<uint64_t>, std::vector<uint8_t> >
              &tmp_idx_v = tmp_idx[v];
          index_t &idx_v = index_[inv[v]];

          // Prefetch
          _mm_prefetch(&idx_v.bpspt_d[0], _MM_HINT_T0);
          _mm_prefetch(&idx_v.bpspt_s[0][0], _MM_HINT_T0);
          _mm_prefetch(&tmp_idx_v.first[0], _MM_HINT_T0);
          _mm_prefetch(&tmp_idx_v.second[0], _MM_HINT_T0);

          // Prune?
          if (usd[v]) continue;
          for (uint64_t i = 0; i < kNumBitParallelRoots; ++i) {
            uint64_t td = idx_r.bpspt_d[i] + idx_v.bpspt_d[i];
            if (td - 2 <= d) {
              td +=
                  (idx_r.bpspt_s[i][0] & idx_v.bpspt_s[i][0]) ? -2 :
                  ((idx_r.bpspt_s[i][0] & idx_v.bpspt_s[i][1]) |
                   (idx_r.bpspt_s[i][1] & idx_v.bpspt_s[i][0]))
                  ? -1 : 0;
              if (td <= d) goto pruned;
            }
          }
          for (size_t i = 0; i < tmp_idx_v.first.size(); ++i) {
            uint64_t w = tmp_idx_v.first[i];
            uint64_t td = tmp_idx_v.second[i] + dst_r[w];
            if (td <= d) goto pruned;
          }

          // Traverse
          tmp_idx_v.first .back() = r;
          tmp_idx_v.second.back() = d;
          tmp_idx_v.first .push_back(V);
          tmp_idx_v.second.push_back(INF8);
          for (size_t i = 0; i < adj[v].size(); ++i) {
            uint64_t w = adj[v][i];
            if (!vis[w]) {
              que[que_h++] = w;
              vis[w] = true;
            }
          }

          profiler.add_label(1);
//          if (profiler.get_label_count() % 1E6 == 0) {
//        	  profiler.print("Label: ", profiler.get_label_count(), " Time: ", profiler.time_click(), "\n");
//          }

       pruned:
          {}
        }

        que_t0 = que_t1;
        que_t1 = que_h;

//        profiler.print("d: ", d, " que_t0: ", que_t0, " que_h: ", que_h, "\n");
        //if (profiler.equal_values(d, uint64_t(INF8))) {
        //	profiler.print("d: ", d, " que_t0: ", que_t0, " que_h: ", que_h, "\n");
        //	if (que_t0 >= que_h) {
        //		exit(2);
        //	}
        //}
      }

      for (uint64_t i = 0; i < que_h; ++i) vis[que[i]] = false;
      for (size_t i = 0; i < tmp_idx_r.first.size(); ++i) {
        dst_r[tmp_idx_r.first[i]] = INF8;
      }
      usd[r] = true;
//      printf("The %u BFS finished (%d vertices).\n", profiler.bfs_click(), que_h);
      profiler.print(profiler.bfs_click(), " BFS finished. ", que_h, " vertices. ", profiler.get_label_count(), " ", profiler.time_click(), "\n");
    }

    for (uint64_t v = 0; v < V; ++v) {
      uint64_t k = tmp_idx[v].first.size();
      index_[inv[v]].spt_v = (uint64_t*)memalign(64, k * sizeof(uint64_t));
      index_[inv[v]].spt_d = (uint8_t *)memalign(64, k * sizeof(uint8_t ));
      if (!index_[inv[v]].spt_v || !index_[inv[v]].spt_d) {
        Free();
        return false;
      }
      for (uint64_t i = 0; i < k; ++i) index_[inv[v]].spt_v[i] = tmp_idx[v].first[i];
      for (uint64_t i = 0; i < k; ++i) index_[inv[v]].spt_d[i] = tmp_idx[v].second[i];
      tmp_idx[v].first.clear();
      tmp_idx[v].second.clear();
    }
  }
  profiler.print("PLL finished.\n");

  time_indexing_ += GetCurrentTimeSec();
  return true;
}
// End By Johnpzh

template<uint64_t kNumBitParallelRoots>
uint64_t PrunedLandmarkLabeling<kNumBitParallelRoots>
// By Johnpzh
::QueryDistance(uint64_t v, uint64_t w)
//::QueryDistance(uint64_t v, uint64_t w) 
// End By Johnpzh
{
  if (v >= num_v_ || w >= num_v_) return v == w ? 0 : INT_MAX;

  const index_t &idx_v = index_[v];
  const index_t &idx_w = index_[w];
  uint64_t d = INF8;

  _mm_prefetch(&idx_v.spt_v[0], _MM_HINT_T0);
  _mm_prefetch(&idx_w.spt_v[0], _MM_HINT_T0);
  _mm_prefetch(&idx_v.spt_d[0], _MM_HINT_T0);
  _mm_prefetch(&idx_w.spt_d[0], _MM_HINT_T0);

  for (uint64_t i = 0; i < kNumBitParallelRoots; ++i) {
    uint64_t td = idx_v.bpspt_d[i] + idx_w.bpspt_d[i];
    if (td - 2 <= d) {
      td +=
          (idx_v.bpspt_s[i][0] & idx_w.bpspt_s[i][0]) ? -2 :
          ((idx_v.bpspt_s[i][0] & idx_w.bpspt_s[i][1]) | (idx_v.bpspt_s[i][1] & idx_w.bpspt_s[i][0]))
          ? -1 : 0;

      if (td < d) d = td;
    }
  }
  for (uint64_t i1 = 0, i2 = 0; ; ) {
    uint64_t v1 = idx_v.spt_v[i1], v2 = idx_w.spt_v[i2];
    if (v1 == v2) {
      if (v1 == num_v_) break;  // Sentinel
      uint64_t td = idx_v.spt_d[i1] + idx_w.spt_d[i2];
      if (td < d) d = td;
      ++i1;
      ++i2;
    } else {
      i1 += v1 < v2 ? 1 : 0;
      i2 += v1 > v2 ? 1 : 0;
    }
  }

  if (d >= INF8 - 2) d = INT_MAX;
  return d;
}

template<uint64_t kNumBitParallelRoots>
bool PrunedLandmarkLabeling<kNumBitParallelRoots>
::LoadIndex(const char *filename) {
  std::ifstream ifs(filename);
  return ifs && LoadIndex(ifs);
}

template<uint64_t kNumBitParallelRoots>
bool PrunedLandmarkLabeling<kNumBitParallelRoots>
::LoadIndex(std::istream &ifs) {
  Free();

  int32_t num_v, num_bpr;
  ifs.read((char*)&num_v,   sizeof(num_v));
  ifs.read((char*)&num_bpr, sizeof(num_bpr));
  num_v_ = num_v;
  if (ifs.bad() || kNumBitParallelRoots != num_bpr) {
    num_v_ = 0;
    return false;
  }

  index_ = (index_t*)memalign(64, num_v * sizeof(index_t));
  if (index_ == NULL) {
    num_v_ = 0;
    return false;
  }
  for (uint64_t v = 0; v < num_v_; ++v) {
    index_[v].spt_v = NULL;
    index_[v].spt_d = NULL;
  }

  for (uint64_t v = 0; v < num_v_; ++v) {
    index_t &idx = index_[v];

    for (uint64_t i = 0; i < kNumBitParallelRoots; ++i) {
      ifs.read((char*)&idx.bpspt_d[i]   , sizeof(idx.bpspt_d[i]   ));
      ifs.read((char*)&idx.bpspt_s[i][0], sizeof(idx.bpspt_s[i][0]));
      ifs.read((char*)&idx.bpspt_s[i][1], sizeof(idx.bpspt_s[i][1]));
    }

    int32_t s;
    ifs.read((char*)&s, sizeof(s));
    if (ifs.bad()) {
      Free();
      return false;
    }

    idx.spt_v = (uint64_t*)memalign(64, s * sizeof(uint64_t));
    idx.spt_d = (uint8_t *)memalign(64, s * sizeof(uint8_t ));
    if (!idx.spt_v || !idx.spt_d) {
      Free();
      return false;
    }

    for (int32_t i = 0; i < s; ++i) {
      ifs.read((char*)&idx.spt_v[i], sizeof(idx.spt_v[i]));
      ifs.read((char*)&idx.spt_d[i], sizeof(idx.spt_d[i]));
    }
  }

  return ifs.good();
}

template<uint64_t kNumBitParallelRoots>
bool PrunedLandmarkLabeling<kNumBitParallelRoots>
::StoreIndex(const char *filename) {
  std::ofstream ofs(filename);
  return ofs && StoreIndex(ofs);
}

template<uint64_t kNumBitParallelRoots>
bool PrunedLandmarkLabeling<kNumBitParallelRoots>
::StoreIndex(std::ostream &ofs) {
  uint64_t num_v = num_v_, num_bpr = kNumBitParallelRoots;
  ofs.write((const char*)&num_v,   sizeof(num_v));
  ofs.write((const char*)&num_bpr, sizeof(num_bpr));

  for (uint64_t v = 0; v < num_v_; ++v) {
    index_t &idx = index_[v];

    for (uint64_t i = 0; i < kNumBitParallelRoots; ++i) {
      int8_t d = idx.bpspt_d[i];
      uint64_t a = idx.bpspt_s[i][0];
      uint64_t b = idx.bpspt_s[i][1];
      ofs.write((const char*)&d, sizeof(d));
      ofs.write((const char*)&a, sizeof(a));
      ofs.write((const char*)&b, sizeof(b));
    }

    int32_t s;
    for (s = 1; idx.spt_v[s - 1] != num_v; ++s) continue;  // Find the sentinel
    ofs.write((const char*)&s, sizeof(s));
    for (int32_t i = 0; i < s; ++i) {
      int32_t l = idx.spt_v[i];
      int8_t  d = idx.spt_d[i];
      ofs.write((const char*)&l, sizeof(l));
      ofs.write((const char*)&d, sizeof(d));
    }
  }

  return ofs.good();
}

template<uint64_t kNumBitParallelRoots>
void PrunedLandmarkLabeling<kNumBitParallelRoots>
::Free() {
  for (uint64_t v = 0; v < num_v_; ++v) {
    free(index_[v].spt_v);
    free(index_[v].spt_d);
  }
  free(index_);
  index_ = NULL;
  num_v_ = 0;
}

template<uint64_t kNumBitParallelRoots>
void PrunedLandmarkLabeling<kNumBitParallelRoots>
::PrintStatistics() {
  std::cout << "load time: "     << time_load_     << " seconds" << std::endl;
  std::cout << "indexing time: " << time_indexing_ << " seconds" << std::endl;

  double s = 0.0;
  for (uint64_t v = 0; v < num_v_; ++v) {
    for (uint64_t i = 0; index_[v].spt_v[i] != uint64_t(num_v_); ++i) {
      ++s;
    }
  }
  s /= num_v_;
  std::cout << "bit-parallel label size: "   << kNumBitParallelRoots << std::endl;
  std::cout << "average normal label size: " << s << std::endl;
}

#endif  // PRUNED_LANDMARK_LABELING_H_
