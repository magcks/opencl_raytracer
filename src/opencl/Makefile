# For debugging purposes, set -O0 and -g.
# For production use, set -O3.
#CC = g++
#
##COMMON_C_FLAGS = -Wall -std=c99 -O3 -mavx -l OpenCL 
#COMMON_C_FLAGS = -O3 -g -Wall -pedantic
#COMMON_LD_FLAGS = -lm
#
#SOURCES := $(wildcard [^_]*.cc)
#OBJECTS := ${SOURCES:.cc=.o}
#BINARY = test
#
#all: $(OBJECTS)
#	$(CC) $(COMMON_C_FLAGS) -o $(BINARY) $(OBJECTS)  $(CLI_LD_FLAGS)
#
#clean:
#	$(RM) $(BINARY) $(OBJECTS)


CXXFLAGS = -O3 -g -Wall -pedantic -msse3
LDLIBS = -lOpenCL
CXX = g++
COMPILE.cc = ${CXX} ${CXXFLAGS} ${CPPFLAGS} -c

SOURCES := $(wildcard [^_]*.cc)
OBJECTS := ${SOURCES:.cc=.o}
BINARY = raytrace

all: $(OBJECTS)
	$(CXX) -o $(BINARY) $(OBJECTS) $(LDLIBS)

%.o: %.cc
	${COMPILE.cc} -o $@ $<

clean:
	$(RM) $(BINARY) $(OBJECTS)
