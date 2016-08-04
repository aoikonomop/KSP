
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

#ifndef MTP_TRACKER_H
#define MTP_TRACKER_H

#include <iostream>

using namespace std;

#include "misc.h"
#include "mtp_graph.h"

class MTPTracker {
  MTPGraph *_graph;

  // The edges will be ordered as follows: First the nb_locations *
  // nb_time_steps edges inside the node pairs, which will have
  // lengths equal to the opposite of the detection scores, then the
  // edges between these node pairs, and finally the edges from source
  // and to sink.
  scalar_t *_edge_lengths;

  int early_pair_node(int t, int l);
  int late_pair_node(int t, int l);

public:

  // The spatial structure
  int nb_locations, nb_time_steps;
  int **allowed_motions;
  int **entrances, **exits;

  // The detection scores at each location and time
  scalar_t **detection_scores;

  MTPTracker();
  ~MTPTracker();

  void allocate(int nb_time_steps, int nb_locations);
  void free();

  void write(ostream *os);
  void read(istream *is);
  void write_trajectories(ostream *os);

  // Build or print the graph needed for the tracking per se

  void build_graph();
  void print_graph_dot(ostream *os);

  // Compute the optimal set of trajectories

  void track();

  // Read-out of the optimal trajectories

  int nb_trajectories();
  scalar_t trajectory_score(int k);
  int trajectory_entrance_time(int k);
  int trajectory_duration(int k);
  int trajectory_location(int k, int time_from_entry);
};

#endif
