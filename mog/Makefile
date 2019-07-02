CXX=g++
CXXFLAGS=-Wall -O -pipe  `pkg-config --cflags opencv`
LDFLAGS=-lm `pkg-config --libs opencv`

OBJ=main.o 

all: clean main

main: $(OBJ)
	$(CXX) $? -o $@ $(LDFLAGS) 

clean:
	rm -f ./*.o ./main

fix:
	ci -l *.cc *.hh Makefile run.sh README

#	ci -l *.h 

# $Id: Makefile,v 1.2 2015/05/22 11:11:42 mechanoid Exp mechanoid $
