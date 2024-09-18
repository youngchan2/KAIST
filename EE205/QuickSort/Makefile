CC = gcc
CFLAGS = -W -Wall

all: test productRanking

test: quick.o test.o
	$(CC) $(CFLAGS) -o test quick.o test.o

productRanking: productRanking.o
	$(CC) $(CFLAGS) -o productRanking productRanking.o

%.o: %.c
	$(CC) $(CFLAGS) $< -c

quick.o: quick.c quick.h

test.o: test.c

clean:
	@rm *.o test productRanking