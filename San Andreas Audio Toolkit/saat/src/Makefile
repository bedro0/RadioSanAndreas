# The San Andreas Audio Toolkit (SAAT): Makefile
# Copyright (C) 2005 Dave Schmitt <pdescobar@comcast.net>
#
################
# Configurations
#
# Edit PLATFORM to suit your needs or call make as "make 'PLATFORM=whatever'"
# If none of the predefined configs work for you, you can edit them or choose
# 'custom' and add appropriate options in the custom block.
#
# Predefined PLATFORM configurations:
#  cygwin -- .exe; POSIX pathing but WIN32 filenames; compiles with -pedantic
#  mingw -- .exe; WIN32 pathing, filenames and mkdir(); /mingw in search path
#  posix -- POSIX paths; unrestricted filenames; calls posix_fadvise; -pedantic
#  release -- very specific cygwin/mingw hybrid for the WIN32 binary release
#  custom -- blank; just insert your own values in the first section.
PLATFORM = release
#
# Useful defines for PLATFORM_CCFLAGS if using custom configuration:
#  -DWIN32 -- uses modeless mkdir and uses WIN32 paths and filenames as below.
#  -DWIN32_PATHS -- Backslash (\) directory separator, compensates for 
#                   potential drive names with colons (e.g. c:\foo)
#  -DWIN32_FILENAMES -- Sanitize output filenames to get rid of <>\|/*:"?
#  -DPOSIX_FADVISE -- enables posix_fadvise call to advise kernel not
#                     to cache some input data.
# Yeah, this is probably inefficient. Oh well.
ifeq ($(PLATFORM),custom)
  PLATFORM_CCFLAGS= 
  PLATFORM_LDFLAGS= 
  EXE_SUFFIX= 
endif
ifeq ($(PLATFORM),cygwin)
  PLATFORM_CCFLAGS=-DWIN32_FILENAMES -pedantic -I/usr/local/include
  PLATFORM_LDFLAGS=-L/usr/local/lib
  EXE_SUFFIX=.exe
endif
ifeq ($(PLATFORM),mingw)
  PLATFORM_CCFLAGS=-DWIN32 -I/mingw/include
  PLATFORM_LDFLAGS=-L/mingw/lib
  EXE_SUFFIX=.exe
endif
ifeq ($(PLATFORM),posix)
  PLATFORM_CCFLAGS=-DPOSIX_FADVISE -pedantic
  PLATFORM_LDFLAGS= 
  EXE_SUFFIX= 
endif
ifeq ($(PLATFORM),release)
  PLATFORM_CCFLAGS=-DWIN32 -mno-cygwin -I/cygdrive/d/msys/mingw/include
  PLATFORM_LDFLAGS=-mno-cygwin -L/cygdrive/d/msys/mingw/lib -static
  EXE_SUFFIX=.exe
endif

################
# Make Variables

# Compile and Link Options
CXX=g++
CXXFLAGS=-g -ansi -Wall -O2 $(PLATFORM_CCFLAGS)
CC=gcc
CFLAGS=$(CXXFLAGS)
STREAM_LIBS=$(PLATFORM_LDFLAGS) -logg -lvorbis
SFX_LIBS=$(PLATFORM_LDFLAGS) -lsndfile

# Object variables that must be set
MAIN_STREAM_OBJ=saat_stream.o
MAIN_SFX_OBJ=saat_sfx.o
COMMON_OBJS=saat_common.o config.o lookup.o virtual_file.o
STREAM_OBJS=vcedit.o
SFX_OBJS=

# Other variables made from those above
MAIN_OBJS=$(MAIN_STREAM_OBJ) $(MAIN_SFX_OBJ)
STREAM_EXE=$(MAIN_STREAM_OBJ:%.o=%$(EXE_SUFFIX))
SFX_EXE=$(MAIN_SFX_OBJ:%.o=%$(EXE_SUFFIX))
EXES=$(STREAM_EXE) $(SFX_EXE)
STREAM_HEADS=$(STREAM_OBJS:%.o=%.h) 
SFX_HEADS=$(SFX_OBJS:%.o=%.h) 
COMMON_HEADS=$(COMMON_OBJS:%.o=%.h)
OBJECTS=$(COMMON_OBJS) $(SAAT_OBJS) $(SFX_OBJS)
HEADERS=$(OBJECTS:%.o=%.h)

###############
# Build Targets

all: $(EXES)

$(SFX_EXE): $(MAIN_SFX_OBJ) $(COMMON_OBJS) $(SFX_OBJS)
	$(CXX) -o $@ $^ $(SFX_LIBS)

$(STREAM_EXE): $(MAIN_STREAM_OBJ) $(COMMON_OBJS) $(STREAM_OBJS)
	$(CXX) -o $@ $^ $(STREAM_LIBS)

saat_sfx.o: saat_sfx.cpp saat_sfx.h $(SFX_HEADS) $(COMMON_HEADS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

saat_stream.o: saat_stream.cpp saat_stream.h $(STREAM_HEADS) $(COMMON_HEADS)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

saat_common.o: saat_common.cpp saat_common.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

config.o: config.cpp config.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

lookup.o: lookup.cpp lookup.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

vcedit.o: vcedit.c vcedit.h i18n.h utf8.h
	$(CC) $(CFLAGS) -o $@ -c $<

virtual_file.o: virtual_file.cpp virtual_file.h
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJECTS) $(MAIN_OBJS) $(EXES)

.PHONY: all clean release stream_test sfx_test

# These might be useful for active development; certainly not for anything else
release:
	strip -s $(EXES)
	mv $(EXES) ..
	rm -f $(OBJECTS) $(MAIN_OBJS)

stream_test:
	cp ./test/audio/CONFIG/TrakLkup.dat.fixed ./test/audio/CONFIG/TrakLkup.dat
	rm -rf ./test/export
	rm -rf ./test/rfsa
	rm -rf ./test/import/*
	./saat_stream -e ./test/audio/streams/* ./test/export
	./saat_stream -r ./test/audio/streams/* ./test/rfsa ./metadata.conf
	./saat_stream -i ./test/import/AA ./test/export/AA/stream_import.ini ./test/audio/CONFIG/TrakLkup.dat
	./saat_stream -i ./test/import/BEATS ./test/export/BEATS/stream_import.ini ./test/audio/CONFIG/TrakLkup.dat
	diff --binary --brief ./test/audio/CONFIG/TrakLkup.dat ./test/audio/CONFIG/TrakLkup.dat.fixed
	diff --binary --brief ./test/import/AA ./test/audio/streams/AA
	diff --binary --brief ./test/import/BEATS ./test/audio/streams/BEATS

sfx_test:
	cp ./test/audio/CONFIG/BankLkup.dat.orig ./test/audio/CONFIG/BankLkup.dat

