CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = paging_translator

all: $(TARGET)

$(TARGET): paging_translator.c
	$(CC) $(CFLAGS) -o $(TARGET) paging_translator.c

clean:
	rm -f $(TARGET)