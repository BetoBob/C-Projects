CC = gcc
CFLAGS = -Wall -ansi -pedantic -g -lm -std=c99
MAIN = fw
OBJS = huffman.o
all : hencode hdecode

hencode: hencode.o
hdecode: hdecode.o

hencode.o : hencode.c $(OBJS)
	$(CC) $(CFLAGS) -c hencode.c

hdecode.o : hdecode.c $(OBJS)
	$(CC) $(CFLAGS) -c hdecode.c

huffman.o : huffman.h
	$(CC) $(CFLAGS) -c huffman.h

clean :
	rm *.o
