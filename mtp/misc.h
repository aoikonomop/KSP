
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

#ifndef MISC_H
#define MISC_H

#include <stdlib.h>

typedef float scalar_t;

#ifdef DEBUG
#define ASSERT(x) if(!(x)) {                                            \
    std::cerr << "ASSERT FAILED IN " << __FILE__ << ":" << __LINE__ << endl; \
    abort();                                                            \
  }
#else
#define ASSERT(x)
#endif

template<class T>
T **allocate_array(int a, int b) {
  T *whole = new T[a * b];
  T **array = new T *[a];
  for(int k = 0; k < a; k++) {
    array[k] = whole;
    whole += b;
  }
  return array;
}

template<class T>
void deallocate_array(T **array) {
  if(array) {
    delete[] array[0];
    delete[] array;
  }
}

#endif
