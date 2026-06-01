CC = gcc
CFLAGS = -O -Wall -Wstrict-prototypes -Wmissing-prototypes

all: norm1 norm2 algcomp2

norm1: norm1.o machine.o alg.o
	$(CC) norm1.o machine.o alg.o -o norm1
	cp norm1 datafiles

norm2: norm2.o machine.o alg.o
	$(CC) norm2.o machine.o alg.o -lm -o norm2
	cp norm2 datafiles

algcomp2: algcomp2.o machine.o alg.o
	$(CC) algcomp2.o machine.o alg.o -o algcomp2
	cp algcomp2 datafiles

depend:
	makedepend norm1.c machine.c alg.c norm2.c algcomp2.c

# DO NOT DELETE
