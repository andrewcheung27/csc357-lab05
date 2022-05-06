CC = gcc
CFLAGS = -Wall -pedantic -g -O3


all: mypwd

mypwd: mypwd.o
	$(CC) $(CFLAGS) -o mypwd mypwd.o

mypwd.o: mypwd.c
	$(CC) $(CFLAGS) -c -o mypwd.o mypwd.c


clean:
	rm -f *.o

server:
	ssh acheun29@unix1.csc.calpoly.edu

upload:
	scp -r ../csc357-asgn3 acheun29@unix1.csc.calpoly.edu:csc357

download:
	scp -r acheun29@unix1.csc.calpoly.edu:csc357/csc357-asgn3 /Users/andrewcheung/Documents/Cal\ Poly\ Stuff/csc357

