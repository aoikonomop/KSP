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
//#include <time.h>
//#include <windows.h>
////#include "mtp/getopt.h"
//#include <limits.h>
//#include <string.h>
//
//using namespace std;
//
//#include "mtp_tracker.h"
//
//#define FILENAME_SIZE 1024
//
//struct Global {
//  char trajectory_filename[FILENAME_SIZE];
//  char graph_filename[FILENAME_SIZE];
//  int verbose;
//} global;
//
//void usage(ostream *os) {
//  (*os) << "mtp [-h|--help] [--help-formats] [-v|--verbose] [-t|--trajectory-filename <trajectory filename>] [-g|--graph-filename <graph filename>] [<tracking parameter file>]" << endl;
//  (*os) << endl;
//  (*os) << "The mtp command processes a file containing the description of a topology" << endl;
//  (*os) << "and detection scores, and prints the optimal set of trajectories." << endl;
//  (*os) << endl;
//  (*os) << "If no filename is provided, it reads the parameters from the standard" << endl;
//  (*os) << "input. If no trajectory filename is provided, it writes the result to" << endl;
//  (*os) << "the standard output." << endl;
//  (*os) << endl;
//  (*os) << "Written by Francois Fleuret. (C) Idiap Research Institute, 2012." << endl;
//}
//
//void print_help_formats() {
//  cout << "The tracking parameters the command takes as input have the following" << endl;
//  cout << "format, where L is the number of locations and T is the number of time" << endl;
//  cout << "steps:" << endl;
//  cout << endl;
//  cout << "---------------------------- snip snip -------------------------------" << endl;
//  cout << "  int:L int:T" << endl;
//  cout << endl;
//  cout << "  bool:allowed_motion_from_1_to_1 ... bool:allowed_motion_from_1_to_L" << endl;
//  cout << "  ..." << endl;
//  cout << "  bool:allowed_motion_from_L_to_1 ... bool:allowed_motion_from_L_to_L" << endl;
//  cout << endl;
//  cout << "  bool:entrance_1_1 ... bool:entrance_1_L" << endl;
//  cout << "  ..." << endl;
//  cout << "  bool:entrance_T_1 ... bool:entrance_T_L" << endl;
//  cout << endl;
//  cout << "  bool:exit_1_1 ... bool:exit_1_L" << endl;
//  cout << "  ..." << endl;
//  cout << "  bool:exit_T_1 ... bool:exit_T_L" << endl;
//  cout << endl;
//  cout << "  float:detection_score_1_1 ... float:detection_score_1_L" << endl;
//  cout << "  ..." << endl;
//  cout << "  float:detection_score_T_1 ... float:detection_score_T_L" << endl;
//  cout << "---------------------------- snip snip -------------------------------" << endl;
//  cout << endl;
//  cout << "As results, the command writes first the number of trajectories," << endl;
//  cout << "followed by one line per trajectory with the following structure:" << endl;
//  cout << endl;
//  cout << "---------------------------- snip snip -------------------------------" << endl;
//  cout << "  int:traj_number int:entrance_time int:duration float:score int:location_1 ... int:location_duration" << endl;
//  cout << "---------------------------- snip snip -------------------------------" << endl;
//}
//
//scalar_t diff_in_second(struct timeval *start, struct timeval *end) {
//  return
//    scalar_t(end->tv_sec - start->tv_sec) +
//    scalar_t(end->tv_usec - start->tv_usec)/1000000;
//}
//
//void do_tracking(istream *in_tracker) {
//  timeval start_time, end_time;
//  MTPTracker *tracker = new MTPTracker();
//
//  if(global.verbose) { cout << "Reading the tracking parameters." << endl; }
//  tracker->read(in_tracker);
//
//  if(global.verbose) {
//    cout << "Building the graph ... "; cout.flush();
//    //gettimeofday(&start_time, 0);
//  }
//  tracker->build_graph();
//  if(global.verbose) {
//    //gettimeofday(&end_time, 0);
//    cout << "done (" << diff_in_second(&start_time, &end_time) << "s)." << endl;
//  }
//
//  if(global.verbose) {
//    cout << "Tracking ... "; cout.flush();
//    //gettimeofday(&start_time, 0);
//  }
//  tracker->track();
//  if(global.verbose) {
//    //gettimeofday(&end_time, 0);
//    cout << "done (" << diff_in_second(&start_time, &end_time) << "s)." << endl;
//  }
//
//  if(global.trajectory_filename[0]) {
//    ofstream out_traj(global.trajectory_filename);
//    tracker->write_trajectories(&out_traj);
//    if(global.verbose) { cout << "Wrote " << global.trajectory_filename << "." << endl; }
//  } else {
//    tracker->write_trajectories(&cout);
//  }
//
//  if(global.graph_filename[0]) {
//    ofstream out_dot(global.graph_filename);
//    tracker->print_graph_dot(&out_dot);
//    if(global.verbose) { cout << "Wrote " << global.graph_filename << "." << endl; }
//  }
//
//  delete tracker;
//}
//
//enum
//{
//  OPT_HELP_FORMATS = CHAR_MAX + 1
//};
//
////static struct option long_options[] = {
////  { "trajectory-file", 1, 0, 't' },
////  { "graph-file", 1, 0, 'g' },
////  { "help", no_argument, 0, 'h' },
////  { "verbose", no_argument, 0, 'v' },
////  { "help-formats", no_argument, 0, OPT_HELP_FORMATS },
////  { 0, 0, 0, 0 }
////};
//
//int main(int argc, char *argv[]) {
//  int c;
//  int error = 0, show_help = 0;
//
//  strncpy(global.trajectory_filename, argv[2], FILENAME_SIZE);
//  strncpy(global.graph_filename, argv[3], FILENAME_SIZE);
//  global.verbose = atoi(argv[4]);
//
//  ifstream *file_in_tracker = new ifstream(argv[1]);
//  if (file_in_tracker->good()) {
//	  do_tracking(file_in_tracker);
//  }
//  else {
//	  cerr << "Can not open " << argv[1] << endl;
//	  exit(EXIT_FAILURE);
//  }
//  
//  /*strncpy(global.trajectory_filename, "", FILENAME_SIZE);
//  strncpy(global.graph_filename, "", FILENAME_SIZE);
//  global.verbose = 0;
//
//  while ((c = getopt(argc, argv, "t:g:hv")) != -1) {
//
//    switch(c) {
//
//    case 't':
//      strncpy(global.trajectory_filename, optarg, FILENAME_SIZE);
//      break;
//
//    case 'g':
//      strncpy(global.graph_filename, optarg, FILENAME_SIZE);
//      break;
//
//    case 'h':
//      show_help = 1;
//      break;
//
//    case OPT_HELP_FORMATS:
//      print_help_formats();
//      exit(EXIT_SUCCESS);
//      break;
//
//    case 'v':
//      global.verbose = 1;
//      break;
//
//    default:
//      error = 1;
//      break;
//    }
//  }*/
//
//  /*if(error) {
//    usage(&cerr);
//    exit(EXIT_FAILURE);
//  }
//
//  if(show_help) {
//    usage(&cout);
//    exit(EXIT_SUCCESS);
//  }*/
//
//  /*if(optind < argc) {
//    ifstream *file_in_tracker = new ifstream(argv[optind]);
//    if(file_in_tracker->good()) {
//      do_tracking(file_in_tracker);
//    } else {
//      cerr << "Can not open " << argv[optind] << endl;
//      exit(EXIT_FAILURE);
//    }
//    delete file_in_tracker;
//  } else {
//    do_tracking(&cin);
//  }*/
//
//  exit(EXIT_SUCCESS);
//}
