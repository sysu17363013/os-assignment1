# makefile for prod2 and cons2
#

CC = gcc
CFLAG = -g -c -Wall
 
all: prod cons dph
 
%.o:%.c
	$(CC) $(CFLAG) -c $*.c -lm -lpthread
 
prod:prod.o
		$(CC) -o $@ prod.o -lm -lpthread
 
cons:cons.o
		$(CC) -o $@ cons.o -lm -lpthread

dph: dph.o
	$(CC) -Wall -o dph dph.o -lpthread

dph.o: dph.c
	$(CC) -Wall -c dph.c -lpthread

dph:dph.o
		$(CC) -o $@ dph.o -lm -lpthread

dph: dph
dph: dph.o
	$(CC) $(CFLAGS) -o dph dph.o -lm -lpthread

dph.o: dph.c
	$(CC) $(CFLAGS) -c dph.c -lm -lpthread

prod:prod
prod: prod.o
	$(CC) $(CFLAGS) -o prod prod.o -lm -lpthread

dph.o: dph.c
	$(CC) $(CFLAGS) -c prod.c -lm -lpthread

cons:cons
cons: cons.o
	$(CC) $(CFLAGS) -o cons cons.o -lm -lpthread

cons.o: cons.c
	$(CC) $(CFLAGS) -c cons.c -lm -lpthread

clean:
	rm -rf *.o
