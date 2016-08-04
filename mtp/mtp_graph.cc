
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

#include "mtp_graph.h"

#include <cmath>
#include <float.h>

using namespace std;

class Edge {
public:
  int occupied;
  scalar_t length, positivized_length;
  Vertex *origin_vertex, *terminal_vertex;

  // These fields are used for the linked list of a vertex's leaving
  // edge list. We have to do insertions / deletions.
  Edge *next_leaving_edge, *pred_leaving_edge;

  inline void invert();
};

class Vertex {
public:
  scalar_t distance_from_source;
  Edge *pred_edge_toward_source;

  Edge *leaving_edge_list_root;
  Vertex **heap_slot;

  Vertex();

  inline void add_leaving_edge(Edge *e);
  inline void del_leaving_edge(Edge *e);
  inline void decrease_distance_in_heap(Vertex **heap);
  inline void increase_distance_in_heap(Vertex **heap, Vertex **heap_bottom);
};

//////////////////////////////////////////////////////////////////////

void Edge::invert() {
  length = - length;
  positivized_length = - positivized_length;
  origin_vertex->del_leaving_edge(this);
  terminal_vertex->add_leaving_edge(this);
  swap(terminal_vertex, origin_vertex);
}

//////////////////////////////////////////////////////////////////////

Vertex::Vertex() {
  leaving_edge_list_root = 0;
}

void Vertex::add_leaving_edge(Edge *e) {
  e->next_leaving_edge = leaving_edge_list_root;
  e->pred_leaving_edge = 0;
  if(leaving_edge_list_root) {
    leaving_edge_list_root->pred_leaving_edge = e;
  }
  leaving_edge_list_root = e;
}

void Vertex::del_leaving_edge(Edge *e) {
  if(e == leaving_edge_list_root) {
    leaving_edge_list_root = e->next_leaving_edge;
  }
  if(e->pred_leaving_edge) {
    e->pred_leaving_edge->next_leaving_edge = e->next_leaving_edge;
  }
  if(e->next_leaving_edge) {
    e->next_leaving_edge->pred_leaving_edge = e->pred_leaving_edge;
  }
}

void Vertex::decrease_distance_in_heap(Vertex **heap) {
  Vertex **p, **h;
  h = heap_slot;
  while(1) {
    if(h <= heap) break;
    p = heap + ((h - heap + 1) >> 1) - 1;
    if((*p)->distance_from_source <= distance_from_source) break;
    swap((*p)->heap_slot, heap_slot);
    swap(*p, *h);
    h = p;
  }
}

void Vertex::increase_distance_in_heap(Vertex **heap, Vertex **heap_bottom) {
  Vertex **c1, **c2, **h;
  h = heap_slot;
  while(1) {
    c1 = heap + 2 * (h - heap) + 1;
    if(c1 >= heap_bottom) break;
    c2 = c1 + 1;
    if((*c1)->distance_from_source < distance_from_source) {
      if(c2 < heap_bottom && (*c2)->distance_from_source < (*c1)->distance_from_source) {
        swap((*c2)->heap_slot, heap_slot);
        swap(*c2, *h);
        h = c2;
      } else {
        swap((*c1)->heap_slot, heap_slot);
        swap(*c1, *h);
        h = c1;
      }
    } else {
      if(c2 < heap_bottom && (*c2)->distance_from_source < distance_from_source) {
        swap((*c2)->heap_slot, heap_slot);
        swap(*c2, *h);
        h = c2;
      } else break;
    }
  }
}

//////////////////////////////////////////////////////////////////////

MTPGraph::MTPGraph(int nb_vertices, int nb_edges,
                   int *vertex_from, int *vertex_to,
                   int source, int sink) {
  _nb_vertices = nb_vertices;
  _nb_edges = nb_edges;

  _edges = new Edge[_nb_edges];
  _vertices = new Vertex[_nb_vertices];
  _heap = new Vertex *[_nb_vertices];
  _dp_order = new Vertex *[_nb_vertices];

  _source = &_vertices[source];
  _sink = &_vertices[sink];

  for(int e = 0; e < nb_edges; e++) {
    _vertices[vertex_from[e]].add_leaving_edge(&_edges[e]);
    _edges[e].occupied = 0;
    _edges[e].origin_vertex = &_vertices[vertex_from[e]];
    _edges[e].terminal_vertex = &_vertices[vertex_to[e]];
  }

  for(int v = 0; v < _nb_vertices; v++) {
    _heap[v] = &_vertices[v];
    _vertices[v].heap_slot = &_heap[v];
  }

  paths = 0;
  nb_paths = 0;

  compute_dp_ordering();
}

MTPGraph::~MTPGraph() {
  delete[] _vertices;
  delete[] _dp_order;
  delete[] _heap;
  delete[] _edges;
  for(int p = 0; p < nb_paths; p++) delete paths[p];
  delete[] paths;
}

//////////////////////////////////////////////////////////////////////

void MTPGraph::print(ostream *os) {
  for(int k = 0; k < _nb_edges; k++) {
    Edge *e = &_edges[k];
    (*os) << e->origin_vertex - _vertices
          << " -> "
          << e->terminal_vertex - _vertices
          << " (" << e->length << ")";
    if(e->occupied) { (*os) << " *"; }
    (*os) << endl;
  }
}

void MTPGraph::print_dot(ostream *os) {
  (*os) << "digraph {" << endl;
  (*os) << "        rankdir=\"LR\";" << endl;
  (*os) << "        node [shape=circle,width=0.75,fixedsize=true];" << endl;
  (*os) << "        edge [color=gray,arrowhead=open]" << endl;
  (*os) << "        " << _source - _vertices << " [peripheries=2];" << endl;
  (*os) << "        " << _sink - _vertices << " [peripheries=2];" << endl;
  for(int k = 0; k < _nb_edges; k++) {
    Edge *e = &_edges[k];
    (*os) << "        "
          << e->origin_vertex - _vertices
          << " -> "
          << e->terminal_vertex - _vertices
          << " [";
    if(e->occupied) {
      (*os) << "style=bold,color=black,";
    }
    (*os) << "label=\"" << e->length << "\"];" << endl;
  }
  (*os) << "}" << endl;
}

//////////////////////////////////////////////////////////////////////

void MTPGraph::update_positivized_lengths() {
  for(int k = 0; k < _nb_edges; k++) {
    Edge *e = &_edges[k];
    e->positivized_length +=
      e->origin_vertex->distance_from_source - e->terminal_vertex->distance_from_source;
  }
}

void MTPGraph::force_positivized_lengths() {
#ifdef VERBOSE
  scalar_t residual_error = 0.0;
  scalar_t max_error = 0.0;
#endif
  for(int k = 0; k < _nb_edges; k++) {
    Edge *e = &_edges[k];

    if(e->positivized_length < 0) {
#ifdef VERBOSE
      residual_error -= e->positivized_length;
      max_error = max(max_error, - e->positivized_length);
#endif
      e->positivized_length = 0.0;
    }
  }
#ifdef VERBOSE
  cerr << __FILE__ << ": residual_error " << residual_error << " max_error " << max_error << endl;
#endif
}

void MTPGraph::dp_compute_distances() {
  Vertex *v, *tv;
  Edge *e;
  scalar_t d;

  for(int k = 0; k < _nb_vertices; k++) {
    _vertices[k].distance_from_source = FLT_MAX;
    _vertices[k].pred_edge_toward_source = 0;
  }

  _source->distance_from_source = 0;

  for(int k = 0; k < _nb_vertices; k++) {
    v = _dp_order[k];
    for(e = v->leaving_edge_list_root; e; e = e->next_leaving_edge) {
      d = v->distance_from_source + e->positivized_length;
      tv = e->terminal_vertex;
      if(d < tv->distance_from_source) {
        tv->distance_from_source = d;
        tv->pred_edge_toward_source = e;
      }
    }
  }
}

// This method does not change the edge occupation. It only sets
// properly, for every vertex, the fields distance_from_source and
// pred_edge_toward_source.

void MTPGraph::find_shortest_path() {
  int heap_size;
  Vertex *v, *tv, **last_slot;
  Edge *e;
  scalar_t d;

  for(int k = 0; k < _nb_vertices; k++) {
    _vertices[k].distance_from_source = FLT_MAX;
    _vertices[k].pred_edge_toward_source = 0;
  }

  heap_size = _nb_vertices;
  _source->distance_from_source = 0;
  _source->decrease_distance_in_heap(_heap);

  while(heap_size > 1) {
    // Get the closest to the source
    v = _heap[0];

    // Remove it from the heap (swap it with the last_slot in the heap, and
    // update the distance of that one)
    heap_size--;
    last_slot = _heap + heap_size;
    swap(*_heap, *last_slot); swap((*_heap)->heap_slot, (*last_slot)->heap_slot);
    (*_heap)->increase_distance_in_heap(_heap, last_slot);

    // Now update the neighbors of the node currently closest to the
    // source
    for(e = v->leaving_edge_list_root; e; e = e->next_leaving_edge) {
      d = v->distance_from_source + e->positivized_length;
      tv = e->terminal_vertex;
      if(d < tv->distance_from_source) {
        ASSERT(tv->heap_slot < last_slot);
        tv->distance_from_source = d;
        tv->pred_edge_toward_source = e;
        tv->decrease_distance_in_heap(_heap);
      }
    }
  }
}

void MTPGraph::find_best_paths(scalar_t *lengths) {
  scalar_t shortest_path_length;
  Vertex *v;
  Edge *e;

  for(int e = 0; e < _nb_edges; e++) {
    _edges[e].length = lengths[e];
    _edges[e].occupied = 0;
    _edges[e].positivized_length = _edges[e].length;
  }

  // Compute the distance of all the nodes from the source by just
  // visiting them in the proper DAG ordering we computed when
  // building the graph
  dp_compute_distances();

  do {
    // Use the current distance from the source to make all edge
    // lengths positive
    update_positivized_lengths();
    // Fix numerical errors
    force_positivized_lengths();

    find_shortest_path();

    shortest_path_length = 0.0;

    // Do we reach the sink?
    if(_sink->pred_edge_toward_source) {
      // If yes, compute the length of the best path according to the
      // original edge lengths
      v = _sink;
      while(v->pred_edge_toward_source) {
        shortest_path_length += v->pred_edge_toward_source->length;
        v = v->pred_edge_toward_source->origin_vertex;
      }
      // If that length is negative
      if(shortest_path_length < 0.0) {
#ifdef VERBOSE
        cerr << __FILE__ << ": Found a path of length " << shortest_path_length << endl;
#endif
        // Invert all the edges along the best path
        v = _sink;
        while(v->pred_edge_toward_source) {
          e = v->pred_edge_toward_source;
          v = e->origin_vertex;
          e->invert();
          // This is the only place where we change the occupations of
          // edges
          e->occupied = 1 - e->occupied;
        }
      }
    }

  } while(shortest_path_length < 0.0);

  // Put back the graph in its original state (i.e. invert edges which
  // have been inverted in the process)
  for(int k = 0; k < _nb_edges; k++) {
    e = &_edges[k];
    if(e->occupied) { e->invert(); }
  }
}

int MTPGraph::retrieve_one_path(Edge *e, Path *path, int *used_edges) {
  Edge *f, *next = 0;
  int l = 0, nb_occupied_next;

  if(path) {
    path->nodes[l++] = int(e->origin_vertex - _vertices);
    path->length = e->length;
  } else l++;

  while(e->terminal_vertex != _sink) {
    if(path) {
      path->nodes[l++] = int(e->terminal_vertex - _vertices);
      path->length += e->length;
    } else l++;

    nb_occupied_next = 0;
    for(f = e->terminal_vertex->leaving_edge_list_root; f; f = f->next_leaving_edge) {
      if(f->occupied && !used_edges[f - _edges]) {
        nb_occupied_next++; next = f;
      }
    }

#ifdef DEBUG
    if(nb_occupied_next == 0) {
      cerr << __FILE__ << ": retrieve_one_path: Non-sink end point." << endl;
      abort();
    }
#endif

    if(path) { used_edges[next - _edges] = 1; }

    e = next;
  }

  if(path) {
    path->nodes[l++] = int(e->terminal_vertex - _vertices);
    path->length += e->length;
  } else l++;

  return l;
}

//////////////////////////////////////////////////////////////////////

void MTPGraph::compute_dp_ordering() {
  Vertex *v;
  Edge *e;
  int ntv;

  // This method orders the nodes by putting first the ones with no
  // predecessors, then going on adding nodes whose predecessors have
  // all been already added. Computing the distances from the source
  // by visiting nodes in that order is equivalent to DP.

  int *nb_predecessors = new int[_nb_vertices];

  Vertex **already_processed = _dp_order, **front = _dp_order, **new_front = _dp_order;

  for(int k = 0; k < _nb_vertices; k++) {
    nb_predecessors[k] = 0;
  }

  for(int k = 0; k < _nb_vertices; k++) {
    v = &_vertices[k];
    for(e = v->leaving_edge_list_root; e; e = e->next_leaving_edge) {
      ntv = int(e->terminal_vertex - _vertices);
      nb_predecessors[ntv]++;
    }
  }

  for(int k = 0; k < _nb_vertices; k++) {
    if(nb_predecessors[k] == 0) {
      *(front++) = _vertices + k;
    }
  }

  while(already_processed < front) {
    // Here, nodes before already_processed can be ignored, nodes
    // before front were set to 0 predecessors during the previous
    // iteration. During this new iteration, we have to visit the
    // successors of these ones only, since they are the only ones
    // which may end up with no predecessors.
    new_front = front;
    while(already_processed < front) {
      v = *(already_processed++);
      for(e = v->leaving_edge_list_root; e; e = e->next_leaving_edge) {
        ntv = int(e->terminal_vertex - _vertices);
        nb_predecessors[ntv]--;
        ASSERT(nb_predecessors[ntv] >= 0);
        if(nb_predecessors[ntv] == 0) {
          *(new_front++) = e->terminal_vertex;
        }
      }
    }
    front = new_front;
  }

  if(already_processed < _dp_order + _nb_vertices) {
    cerr << __FILE__ << ": The graph is not a DAG." << endl;
    abort();
  }

  delete[] nb_predecessors;
}

//////////////////////////////////////////////////////////////////////

void MTPGraph::retrieve_disjoint_paths() {
  Edge *e;
  int p, l;
  int *used_edges;

  for(int p = 0; p < nb_paths; p++) delete paths[p];
  delete[] paths;

  nb_paths = 0;
  for(e = _source->leaving_edge_list_root; e; e = e->next_leaving_edge) {
    if(e->occupied) { nb_paths++; }
  }

  paths = new Path *[nb_paths];
  used_edges = new int[_nb_edges];
  for(int e = 0; e < _nb_edges; e++) {
    used_edges[e] = 0;
  }

  p = 0;
  for(e = _source->leaving_edge_list_root; e; e = e->next_leaving_edge) {
    if(e->occupied && !used_edges[e - _edges]) {
      l = retrieve_one_path(e, 0, used_edges);
      paths[p] = new Path(l);
      retrieve_one_path(e, paths[p], used_edges);
      used_edges[e - _edges] = 1;
      p++;
    }
  }

  delete[] used_edges;
}
