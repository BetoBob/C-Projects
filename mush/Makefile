CC = gcc
CFLAGS = -Wall -pedantic -g -lm -std=gnu99
MAIN = mush
OBJS = parseline.o hmush.o

mush: mush.o

mush.o : mush.c $(OBJS)
	$(CC) $(CFLAGS) -c mush.c

parseline.o : parseline.c hmush.o
	$(CC) $(CFLAGS) -c parseline.c

hmush.o : mush.h
	$(CC) $(CFLAGS) -c mush.h

handin:
	handin getaylor-grader 357hw6-03 mush mush.c mush.h parseline.c README Makefile

clean :
	rm *.o *.gch
