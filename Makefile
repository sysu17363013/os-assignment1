CC = gcc
CFLAG = -g -c -Wall
 
all: prod cons dph

dph: dph.o
	$(CC) -Wall -o dph dph.o -lpthread
dph.o: dph.c
	$(CC) -Wall -c dph.c -lpthread

cons: cons.o
	$(CC)  -o cons cons.o -lm -lpthread
cons.o: cons.c
	$(CC) $(CFLAG) -c cons.c -lm -lpthread

prod: prod.o
	$(CC)  -o prod prod.o -lm -lpthread
prod.o: prod.c
	$(CC) $(CFLAG) -c prod.c -lm -lpthread

dph: dph
dph: dph.o
	$(CC) -Wall -o dph dph.o -lpthread
dph.o: dph.c
	$(CC) -Wall -c dph.c -lpthread

prod:prod
prod: prod.o
	$(CC) $(CFLAGS) -o prod prod.o -lm -lpthread
prod.o: prod.c
	$(CC) $(CFLAGS) -c prod.c -lm -lpthread

cons:cons
cons: cons.o
	$(CC) $(CFLAGS) -o cons cons.o -lm -lpthread
cons.o: cons.c
	$(CC) $(CFLAGS) -c cons.c -lm -lpthread

clean:
	rm -rf *.o
