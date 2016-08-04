
# mtp is the ``Multi Tracked Paths'', an implementation of the
# k-shortest paths algorithm for multi-target tracking.
#
# Copyright (c) 2012 Idiap Research Institute, http://www.idiap.ch/
# Written by Francois Fleuret <francois.fleuret@idiap.ch>
#
# This file is part of mtp.
#
# mtp is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# mtp is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
# License for more details.
#
# You should have received a copy of the GNU General Public License
# along with selector.  If not, see <http://www.gnu.org/licenses/>.

ifeq ($(STATIC),yes)
  LDFLAGS=-static -lm
else
  LDFLAGS=-lm
endif

ifeq ($(DEBUG),yes)
  OPTIMIZE_FLAG = -ggdb3 -DDEBUG -fno-omit-frame-pointer
else
  OPTIMIZE_FLAG = -ggdb3 -O3
endif

ifeq ($(VERBOSE),yes)
  VERBOSE_FLAG = -DVERBOSE
endif

ifeq ($(PROFILE),yes)
  PROFILE_FLAG = -pg
endif

CXXFLAGS = -Wconversion -Wall $(OPTIMIZE_FLAG) $(PROFILE_FLAG) $(VERBOSE_FLAG)

all: mtp mtp_example

mtp: \
	path.o \
	mtp_graph.o \
	mtp_tracker.o \
	mtp.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

mtp_example: \
	path.o \
	mtp_graph.o \
	mtp_tracker.o \
	mtp_example.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

Makefile.depend: *.h *.cc Makefile
	$(CC) $(CXXFLAGS) -M *.cc > Makefile.depend

clean:
	\rm -f mtp mtp_example *.o Makefile.depend

-include Makefile.depend
