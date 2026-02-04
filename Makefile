CC = gcc
CFLAGS = -Wall -Wextra

TARGET = Lab2
SRC = Lab2.c

all:$(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)