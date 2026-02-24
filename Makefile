CC = gcc
CFLAGS = -Wall -Wextra -O2

all: sjf rr

sjf: sjf.c
	$(CC) $(CFLAGS) -o sjf sjf.c

rr: rr.c
	$(CC) $(CFLAGS) -o rr rr.c

clean:
	rm -f sjf rr *.o