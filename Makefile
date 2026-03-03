CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

APP = diego_testapp
OBJS = Diego_testFC.o Diego_libFC.o

all: $(APP)

$(APP): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(APP)