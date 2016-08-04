
                       Multi-Tracked Paths (MTP)
                       -------------------------

* INTRODUCTION

This is a very simple implementation of a variant of the k-shortest
paths algorithm (KSP) applied to multi-target tracking, as described
in

  J. Berclaz, E. Turetken, F. Fleuret, and P. Fua. Multiple Object
  Tracking using K-Shortest Paths Optimization. IEEE Transactions on
  Pattern Analysis and Machine Intelligence (TPAMI), 33(9):1806-1819,
  2011.

This implementation is not the reference implementation used for the
experiments presented in this article. It does not require any
library, and uses a Dijkstra with a Binary Heap for the min-queue,
instead of a Fibonacci heap.

This software package provides two commands:

 - mtp is the generic tool to use in practice. It takes tracking
   parameters as input, and prints the tracked trajectories as
   output. The format for these parameters is given at the bottom of
   this documentation.

 - mtp_example creates a tracking toy example, and runs the tracking
   algorithm on it. It gives an example of how to use MTPTracker on a
   configuration produced dynamically, and produces a test input file
   for the mtp command. If you pass it the "stress" argument, it
   generates a larger and noisier problem.

* INSTALLATION

This software should compile with any C++ compiler. Under a unix-like
environment, just execute

  make
  ./mtp_example

It will create a synthetic dummy example, save its description in
tracker.dat, and print the optimal detected trajectories.

If you now execute

  ./mtp --verbose --trajectory-file result.trj --graph-file graph.dot tracker.dat

It will load the file tracker.dat saved by the previous command, run
the detection, save the detected trajectories in result.trj, and the
underlying graph with occupied edges in graph.dot.

If you do have the graphviz set of tools installed, you can produce a
pdf from the latter with the dot command:

  dot < graph.dot -T pdf -o graph.pdf

* IMPLEMENTATION

The two main classes are MTPGraph and MTPTracker.

The MTPGraph class contains a directed acyclic graph (DAG), with a
length for each edge -- which can be negative -- and has methods to
compute the family of paths in this graph that globally minimizes the
sum of edge lengths.

If there are no path of negative length, this optimal family will be
empty, since the minimum total length you can achieve is zero. Note
that the procedure is similar to that of KSP, in the sense that the
family it computes eventually is globally optimal, even if the
computation is iterative.

The MTPTracker takes as input

 (1) a number of locations and a number of time steps

 (2) a spatial topology composed of

     - the allowed motions between locations (a Boolean flag for each
       pair of locations from/to)

     - the entrances (a Boolean flag for each location and time step)

     - the exits (a Boolean flag for each location and time step)

 (3) a detection score for every location and time, which stands for

             log( P(Y(l,t) = 1 | X) / P(Y(l,t) = 0 | X) )

     where Y is the occupancy of location l at time t and X is the
     available observation. In particular, this score is negative on
     locations where the probability that the location is occupied is
     close to 0, and positive when it is close to 1.

From this parameters, the MTPTracker can compute the best set of
disjoint trajectories consistent with the defined topology, which
maximizes the overall detection score (i.e. the sum of the detection
scores of the nodes visited by the trajectories). In particular, if no
trajectory of total positive detection score exists, this optimal set
of trajectories is empty.

An MTPTracker is a wrapper around an MTPGraph. From the defined
spatial topology and number of time steps, it builds a graph with one
source, one sink, and two nodes per location and time. The edges from
the source or to the sink, or between these pairs of nodes, are of
length zero, and the edges between the two nodes of such a pair have
negative lengths, equal to the opposite of the corresponding detection
scores. This structure ensures that the trajectories computed by the
MTPTracker will be node-disjoint, since the trajectories computed by
the MTPGraph are edge-disjoint.

The file mtp_example.cc gives a very simple usage example of the
MTPTracker class by setting the tracker parameters dynamically, and
running the tracking.

The tracker data file for MTPTracker::read has the following format,
where L is the number of locations and T is the number of time steps:

---------------------------- snip snip -------------------------------
  int:L int:T

  bool:allowed_motion_from_1_to_1 ... bool:allowed_motion_from_1_to_L
  ...
  bool:allowed_motion_from_L_to_1 ... bool:allowed_motion_from_L_to_L

  bool:entrance_1_1 ... bool:entrance_1_L
  ...
  bool:entrance_T_1 ... bool:entrance_T_L

  bool:exit_1_1 ... bool:exit_1_L
  ...
  bool:exit_T_1 ... bool:exit_T_L

  float:detection_score_1_1 ... float:detection_score_1_L
  ...
  float:detection_score_T_1 ... float:detection_score_T_L
---------------------------- snip snip -------------------------------

The method MTPTracker::write_trajectories writes first the number of
trajectories, followed by one line per trajectory with the following
structure

---------------------------- snip snip -------------------------------
  int:traj_number int:entrance_time int:duration float:score int:location_1 ... int:location_duration
---------------------------- snip snip -------------------------------

--
François Fleuret
April 2013
