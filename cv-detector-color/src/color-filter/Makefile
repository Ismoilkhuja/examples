CC=gcc
CFLAGS=-Wall -O -pipe  `pkg-config --cflags opencv`
LDFLAGS=-lm `pkg-config --libs opencv`

OBJ=main.o

main: $(OBJ)
	$(CC) *.o -o $@ $(CFLAGS) $(LDFLAGS) 

clean:
	rm -f ./*.o ./main

fix:
	ci -l *.c Makefile
#	ci -l *.h 

# $Id: Makefile,v 1.2 2014/09/23 14:15:42 mechanoid Exp $
