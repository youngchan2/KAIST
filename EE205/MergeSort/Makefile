CC = gcc
CFLAGS = -W -Wall

all: test test_opt

test: msort.o test.o 
	$(CC) $(CFLAGS) -o test msort.o test.o

test_opt: msort_opt.o test_opt.o 
	$(CC) $(CFLAGS) -o test_opt msort_opt.o test_opt.o

%.o: %.c
	$(CC) $(CFLAGS) $< -c

msort.o: msort.c msort.h

msort_opt.o: msort_opt.c msort_opt.h

clean:
	@rm *.o test test_opt