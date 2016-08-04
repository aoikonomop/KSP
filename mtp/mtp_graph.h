
/*
 *  mtp is the ``Multi Tracked Paths'', an implementation of the
 *  k-shortest paths algorithm for multi-target tracking.
 *
 *  Copyright (c) 2012 Idiap Research Institute, http://www.idiap.ch/
 *  Written by Francois Fleuret <francois.fleuret@idiap.ch>
 *
 *  This file is part of mtp.
 *
 *  mtp is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 *  mtp is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with selector.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MTP_GRAPH_H
#define MTP_GRAPH_H

#include <iostream>

using namespace std;

#include "misc.h"
#include "path.h"

class Vertex;
class Edge;

class MTPGraph {

  // Uses the estimated vertex distances to the source to make all the
  // edge lengths positive, resulting in an identical added value to
  // all the paths from the same initial node to the same final node
  // (in particular from source to sink)
  void update_positivized_lengths();

  // It may happen that numerical errors in update_positivized_lengths
  // make the resulting lengths negative, albeit very small. The
  // following method forces all negative lengths to zero, and prints
  // the total correction when compiled in VERBOSE mode.
  void force_positivized_lengths();

  // Visit the vertices according to _dp_order and update their
  // distance from the source
  void dp_compute_distances();

  // Set in every vertex pred_edge_toward_source correspondingly to
  // the path of shortest length. The current implementation is
  // Dijkstra with a Binary Heap (and not with Fibonnaci heap (yet))
  void find_shortest_path();

  // Follows the path starting on edge e and returns the number of
  // nodes to reach the sink. If path is non-null, stores in it the
  // nodes met along the path, and computes path->length properly.
  int retrieve_one_path(Edge *e, Path *path, int *used_edges);

  int _nb_vertices, _nb_edges;
  Vertex *_source, *_sink;

  Edge *_edges;
  Vertex *_vertices;

  // For Dijkstra
  Vertex **_heap;

  // Updating the distances from the source in that order will work in
  // the original graph (which has to be a DAG)
  Vertex **_dp_order;

  // Fills _dp_order
  void compute_dp_ordering();
public:

  // These variables are filled when retrieve_disjoint_paths is called
  int nb_paths;
  Path **paths;

  MTPGraph(int nb_vertices, int nb_edges, int *vertex_from, int *vertex_to,
           int source, int sink);

  ~MTPGraph();

  // Compute the family of paths with minimum total length, set the
  // edge occupied fields accordingly.
  void find_best_paths(scalar_t *lengths);

  // Retrieve the paths corresponding to the occupied edges, and save
  // the result in the nb_paths and paths fields. If the paths are not
  // node-disjoint, there are multiple families of paths that
  // "explain" the edge occupancies, and this method picks one of them
  // arbitrarily.
  void retrieve_disjoint_paths();

  void print(ostream *os);
  void print_dot(ostream *os);
};

#endif
