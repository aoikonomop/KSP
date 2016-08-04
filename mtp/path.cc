
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

#include "path.h"

Path::Path(int n) {
  nb_nodes = n;
  nodes = new int[nb_nodes];
}

Path::~Path() {
  delete[] nodes;
}
