CC = gcc
CFLAGS = -W -Wall

all: proxy

client: proxy.o
	$(CC) $(CFLAGS) -o $@ proxy.o

%.o : %.c
	$(CC) $(CFLAGS) $< -c

clean:
	@rm *.o bf fw