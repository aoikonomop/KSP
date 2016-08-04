//
///*
// *  mtp is the ``Multi Tracked Paths'', an implementation of the
// *  k-shortest paths algorithm for multi-target tracking.
// *
// *  Copyright (c) 2012 Idiap Research Institute, http://www.idiap.ch/
// *  Written by Francois Fleuret <francois.fleuret@idiap.ch>
// *
// *  This file is part of mtp.
// *
// *  mtp is free software: you can redistribute it and/or modify it
// *  under the terms of the GNU General Public License version 3 as
// *  published by the Free Software Foundation.
// *
// *  mtp is distributed in the hope that it will be useful, but WITHOUT
// *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// *  License for more details.
// *
// *  You should have received a copy of the GNU General Public License
// *  along with selector.  If not, see <http://www.gnu.org/licenses/>.
// *
// */
//
//#include <iostream>
//#include <fstream>
//#include <stdlib.h>
//#include <string.h>
//
//using namespace std;
//
//#include "mtp_tracker.h"
//
////////////////////////////////////////////////////////////////////////
//
//scalar_t noisy_score(scalar_t true_score, scalar_t erroneous_score,
//                     scalar_t score_noise, scalar_t flip_noise) {
//  if(double(rand()) / RAND_MAX < flip_noise) {
//	  scalar_t tmp = erroneous_score + score_noise * (2.0f * scalar_t(double(rand()) / RAND_MAX) - 1.0f);
//	  return tmp;
//  } else {
//	  scalar_t tmp = true_score + score_noise * (2.0f * scalar_t(double(rand()) / RAND_MAX) - 1.0f);
//	  return tmp;
//  }
//}
//
//void create_light_test(MTPTracker *tracker) {
//  int nb_locations = 7;
//  int nb_time_steps = 8;
//  int motion_amplitude = 1;
//
//  tracker->allocate(nb_time_steps, nb_locations);
//
//  // We define the spatial structure by stating what are the possible
//  // motions of targets, and what are the entrances and the exits.
//
//  // Here our example is a 1D space with motions from any location to
//  // any location less than motion_amplitude away, entrance at
//  // location 0 (or in the first time frame, i.e. targets can already
//  // be in the scene when the sequence starts) and exit at location
//  // nb_locations-1 (or from the last time frame, i.e. targets can
//  // still be present when the sequence finishes)
//
//  for(int l = 0; l < nb_locations; l++) {
//    for(int m = 0; m < nb_locations; m++) {
//      tracker->allowed_motions[l][m] = abs(l - m) <= motion_amplitude;
//    }
//  }
//
//  for(int t = 0; t < nb_time_steps; t++) {
//    for(int l = 0; l < nb_locations; l++) {
//      // We allow targets to enter in the first time frame, or in
//      // location 0
//      tracker->entrances[t][l] = (t == 0 || l == 0);
//      // We allow targets to leave from the last time frame, or from
//      // location nb_locations-1
//      tracker->exits[t][l] = (t == nb_time_steps - 1 || l == nb_locations-1);
//    }
//  }
//
//  // We construct the graph corresponding to this structure
//
//  tracker->build_graph();
//
//  // Then, we specify for every location and time step what is the
//  // detection score there.
//
//  scalar_t flip_noise = 0.05f;
//  scalar_t score_noise = 0.0f;
//
//  // We first put a background noise, with negative scores at every
//  // location.
//
//  for(int t = 0; t < nb_time_steps; t++) {
//    for(int l = 0; l < nb_locations; l++) {
//      tracker->detection_scores[t][l] = noisy_score(-1.0, 1.0, score_noise, flip_noise);
//    }
//  }
//
//  // Then we add two targets with a typical tracking local minimum
//  //
//  // * Target A moves from location 0 to the middle, stays there for a
//  //   while, and comes back. It is strongly detected on the first
//  //   half
//  //
//  // * Target B moves from location nb_locations-1 to the middle, stay
//  //   there for a while, and comes back. It is strongly detected on
//  //   the second half
//
//  int la, lb; // Target locations
//  scalar_t sa, sb; // Target detection scores
//  for(int t = 0; t < nb_time_steps; t++) {
//    if(t < nb_time_steps/2) {
//      la = t;
//      lb = nb_locations - 1 - t;
//      sa = noisy_score(10.0, -1.0, score_noise, flip_noise);
//      sb = noisy_score( 1.0, -1.0, score_noise, flip_noise);
//    } else {
//      la = nb_time_steps - 1 - t;
//      lb = t - nb_time_steps + nb_locations;
//      sa = noisy_score( 1.0, -1.0, score_noise, flip_noise);
//      sb = noisy_score(10.0, -1.0, score_noise, flip_noise);
//    }
//
//    if(la > nb_locations/2 - 1) la = nb_locations/2 - 1;
//    if(lb < nb_locations/2 + 1) lb = nb_locations/2 + 1;
//
//    tracker->detection_scores[t][la] = sa;
//    tracker->detection_scores[t][lb] = sb;
//  }
//}
//
//void create_heavy_test(MTPTracker *tracker) {
//  int nb_locations = 100;
//  int nb_time_steps = 1000;
//
//  tracker->allocate(nb_time_steps, nb_locations);
//
//  for(int l = 0; l < nb_locations; l++) {
//    for(int m = 0; m < nb_locations; m++) {
//      tracker->allowed_motions[l][m] = (double(rand()) / RAND_MAX < 0.1);
//    }
//  }
//
//  for(int t = 0; t < nb_time_steps; t++) {
//    for(int l = 0; l < nb_locations; l++) {
//      tracker->entrances[t][l] = double(rand()) / RAND_MAX < 0.01;
//      tracker->exits[t][l] = double(rand()) / RAND_MAX < 0.01;
//    }
//  }
//
//  tracker->build_graph();
//
//  for(int t = 0; t < nb_time_steps; t++) {
//    for(int l = 0; l < nb_locations; l++) {
//      tracker->detection_scores[t][l] = scalar_t(double(rand()) / RAND_MAX) - 0.95f;
//    }
//  }
//}
//
//int main(int argc, char **argv) {
//  int stress_test;
//
//  if(argc == 1) {
//    stress_test = 0;
//  } else if(argc == 2 && strcmp(argv[1], "stress") == 0) {
//    stress_test = 1;
//  } else {
//    cerr << "mtp_examples [stress]" << endl;
//    exit(EXIT_FAILURE);
//  }
//
//  MTPTracker *tracker = new MTPTracker();
//
//  if(stress_test) {
//    create_heavy_test(tracker);
//  } else {
//    create_light_test(tracker);
//  }
//
//  {
//    // Write down the tracker setting, so that we can use it as an
//    // example for the mtp command line
//    ofstream out_tracker("tracker.dat");
//    tracker->write(&out_tracker);
//  }
//
//  // Performs the tracking per se
//
//  tracker->track();
//
//  // Prints the detected trajectories
//
//  for(int t = 0; t < tracker->nb_trajectories(); t++) {
//    cout << "Trajectory "
//         << t
//         << " starting at " << tracker->trajectory_entrance_time(t)
//         << ", duration " << tracker->trajectory_duration(t)
//         << ", score " << tracker->trajectory_score(t)
//         << ", through locations";
//    for(int u = 0; u < tracker->trajectory_duration(t); u++) {
//      cout << " " << tracker->trajectory_location(t, u);
//    }
//    cout << endl;
//  }
//
//  delete tracker;
//
//  exit(EXIT_SUCCESS);
//}
