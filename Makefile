#
#
#   Makefile
#
#   Recipes for building the 'fdvcore' executable.
#
#   Copyright (C) 2018 by Matt Roberts, KK5JY.
#   All rights reserved.
#
#   License: GPL v3 (www.gnu.org)
#
#   Source Dependencies:
#      Codec2 (from freedv.org)
#      rtaudio v4 or v5 (from https://www.music.mcgill.ca/~gary/rtaudio/)
#      
#   Distribution Dependencies:
#      build-essential
#      libasound2-dev
#
#

# installation path
INSTALL_TARGET=/usr/local/bin

# list of libraries
LOCAL_LIBS=-lrtaudio -lcodec2 -lsndfile

# debugging flags
DEBUG=-g -ggdb

# list of targets to build
TARGETS=fdvcore
SMALLDV=smalldv

# C++ standard
CPP_STANDARD=-std=c++11

# default target
all: $(TARGETS)

# template targets
.cpp.o:
	g++ $(CPP_STANDARD) $(CFLAGS) $(CDEBUG) -c $<
.cc.o:
	g++ $(CPP_STANDARD) $(CFLAGS) $(CDEBUG) -c $<

# clean targets
clean:
	rm -f $(TARGETS) *.o *.wav

# remove symbols from targets
strip: all
	for i in $(TARGETS) ; do strip --strip-unneeded $$i ; done

# rebuild target
rebuild: clean all

# source dependencies
OBJECTS=fdvcore.o scdv.o

#
#  primary target
#
fdvcore: $(OBJECTS)
	g++ $(DEBUG) $(CXXFLAGS) -o $@ $(OBJECTS) $(LOCAL_LIBS)

#
#  install target
#
install:
	for i in $(TARGETS) $(SMALLDV); do install -g root -o root -m 0755 $$i $(INSTALL_TARGET) ; done

# DO NOT DELETE

fdvcore.o: stype.h localtypes.h SplitCommand.h scdv.h sc.h FirFilter.h IFilter.h
scdv.o: scdv.h sc.h FirFilter.h IFilter.h localtypes.h
