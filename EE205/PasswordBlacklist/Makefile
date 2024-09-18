CC = gcc
CFLAGS = -W -Wall

blacklist: blacklist.o
	$(CC) $(CFLAGS) -o $@ blacklist.o

%.o : %.c
	$(CC) $(CFLAGS) $< -c

clean:
	@rm *.o blacklist
