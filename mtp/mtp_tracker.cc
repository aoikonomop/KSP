
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

#include "mtp_tracker.h"

#include <iostream>

using namespace std;

void MTPTracker::free() {
  delete[] _edge_lengths;
  delete _graph;
  deallocate_array<scalar_t>(detection_scores);
  deallocate_array<int>(allowed_motions);
  deallocate_array<int>(exits);
  deallocate_array<int>(entrances);
}

void MTPTracker::allocate(int t, int l) {
  free();

  nb_locations = l;
  nb_time_steps = t;

  detection_scores = allocate_array<scalar_t>(nb_time_steps, nb_locations);
  allowed_motions = allocate_array<int>(nb_locations, nb_locations);

  entrances = allocate_array<int>(nb_time_steps, nb_locations);
  exits = allocate_array<int>(nb_time_steps, nb_locations);

  for(int l = 0; l < nb_locations; l++) {
    for(int m = 0; m < nb_locations; m++) {
      allowed_motions[l][m] = 0;
    }
  }

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      detection_scores[t][l] = 0.0;
      entrances[t][l] = 0;
      exits[t][l] = 0;
    }
  }

  _edge_lengths = 0;
  _graph = 0;
}

void MTPTracker::write(ostream *os) {
  (*os) << nb_locations << " " << nb_time_steps << endl;

  (*os) << endl;

  for(int l = 0; l < nb_locations; l++) {
    for(int m = 0; m < nb_locations; m++) {
      (*os) << allowed_motions[l][m];
      if(m < nb_locations - 1) (*os) << " "; else (*os) << endl;
    }
  }

  (*os) << endl;

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      (*os) << entrances[t][l];
      if(l < nb_locations - 1) (*os) << " "; else (*os) << endl;
    }
  }

  (*os) << endl;

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      (*os) << exits[t][l];
      if(l < nb_locations - 1) (*os) << " "; else (*os) << endl;
    }
  }

  (*os) << endl;

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      (*os) << detection_scores[t][l];
      if(l < nb_locations - 1) (*os) << " "; else (*os) << endl;
    }
  }
}

void MTPTracker::read(istream *is) {
  int l = 0, t = 0;

  (*is) >> l >> t;

  allocate(t, l);

  for(int l = 0; l < nb_locations; l++) {
    for(int m = 0; m < nb_locations; m++) {
      (*is) >> allowed_motions[l][m];
    }
  }

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      (*is) >> entrances[t][l];
    }
  }

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      (*is) >> exits[t][l];
    }
  }

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      (*is) >> detection_scores[t][l];
    }
  }
}

void MTPTracker::write_trajectories(ostream *os) {
  (*os) << nb_trajectories() << endl;
  for(int t = 0; t < nb_trajectories(); t++) {
    (*os) << t
         << " " << trajectory_entrance_time(t)
         << " " << trajectory_duration(t)
         << " " << trajectory_score(t);
    for(int u = 0; u < trajectory_duration(t); u++) {
      (*os) << " " << trajectory_location(t, u);
    }
    (*os) << endl;
  }
}

MTPTracker::MTPTracker() {
  nb_locations = 0;
  nb_time_steps = 0;

  detection_scores = 0;
  allowed_motions = 0;

  entrances = 0;
  exits = 0;

  _edge_lengths = 0;
  _graph = 0;
}

MTPTracker::~MTPTracker() {
  free();
}

int MTPTracker::early_pair_node(int t, int l) {
  return 1 + (2 * t + 0) * nb_locations + l;
}

int MTPTracker::late_pair_node(int t, int l) {
  return 1 + (2 * t + 1) * nb_locations + l;
}

void MTPTracker::build_graph() {
  // Delete the existing graph if there was one
  delete[] _edge_lengths;
  delete _graph;

  int nb_motions = 0, nb_exits = 0, nb_entrances = 0;

  for(int l = 0; l < nb_locations; l++) {
    for(int t = 0; t < nb_time_steps; t++) {
      if(exits[t][l]) nb_exits++;
      if(entrances[t][l]) nb_entrances++;
    }
    for(int m = 0; m < nb_locations; m++) {
      if(allowed_motions[l][m]) nb_motions++;
    }
  }

  int nb_vertices = 2 + 2 * nb_time_steps * nb_locations;

  int nb_edges =
    // The edges from the source to the entrances and from the exits
    // to the sink
    nb_exits + nb_entrances +
    // The edges for the motions, between every successive frames
    (nb_time_steps - 1) * nb_motions +
    // The edges inside the duplicated nodes
    nb_locations * nb_time_steps;

  int *node_from = new int[nb_edges];
  int *node_to = new int[nb_edges];

  int source = 0, sink = nb_vertices - 1;
  int e = 0;

  _edge_lengths = new scalar_t[nb_edges];

  // We put the in-node edges first, since these are the ones whose
  // lengths we will have to change before tracking, according to the
  // detection scores

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      node_from[e] = early_pair_node(t, l);
      node_to[e] = late_pair_node(t, l);
      e++;
    }
  }

  // The edges between frames, corresponding to allowed motions

  for(int t = 0; t < nb_time_steps - 1; t++) {
    for(int l = 0; l < nb_locations; l++) {
      for(int k = 0; k < nb_locations; k++) {
        if(allowed_motions[l][k]) {
          node_from[e] = late_pair_node(t, l);
          node_to[e] = early_pair_node(t+1, k);
          _edge_lengths[e] = 0.0;
          e++;
        }
      }
    }
  }

  // The edges from the source to the entrances, and from the exits to
  // the sink

  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      if(entrances[t][l]) {
        node_from[e] = source;
        node_to[e] = early_pair_node(t, l);
        _edge_lengths[e] = 0.0;
        e++;
      }
      if(exits[t][l]) {
        node_from[e] = late_pair_node(t, l);
        node_to[e] = sink;
        _edge_lengths[e] = 0.0;
        e++;
      }
    }
  }

  // We are done, build the graph

  _graph = new MTPGraph(nb_vertices, nb_edges,
                        node_from, node_to,
                        source, sink);

  delete[] node_from;
  delete[] node_to;
}

void MTPTracker::print_graph_dot(ostream *os) {
  int e = 0;
  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      _edge_lengths[e++] = - detection_scores[t][l];
    }
  }
  _graph->print_dot(os);
}

void MTPTracker::track() {
  ASSERT(_graph);

  int e = 0;
  for(int t = 0; t < nb_time_steps; t++) {
    for(int l = 0; l < nb_locations; l++) {
      _edge_lengths[e++] = - detection_scores[t][l];
    }
  }

  _graph->find_best_paths(_edge_lengths);
  _graph->retrieve_disjoint_paths();

#ifdef VERBOSE
  for(int p = 0; p < _graph->nb_paths; p++) {
    Path *path = _graph->paths[p];
    cout << "PATH " << p << " [length " << path->nb_nodes << "] " << path->nodes[0];
    for(int n = 1; n < path->nb_nodes; n++) {
      cout << " -> " << path->nodes[n];
    }
    cout << endl;
  }
#endif
}

int MTPTracker::nb_trajectories() {
  return _graph->nb_paths;
}

scalar_t MTPTracker::trajectory_score(int k) {
  return -_graph->paths[k]->length;
}

int MTPTracker::trajectory_entrance_time(int k) {
  return (_graph->paths[k]->nodes[1] - 1) / (2 * nb_locations);
}

int MTPTracker::trajectory_duration(int k) {
  return (_graph->paths[k]->nb_nodes - 2) / 2;
}

int MTPTracker::trajectory_location(int k, int time_from_entry) {
  return (_graph->paths[k]->nodes[2 * time_from_entry + 1] - 1) % nb_locations;
}
