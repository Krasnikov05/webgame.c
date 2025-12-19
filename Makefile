CC = gcc
CFLAGS = -Wno-unused-result
TARGET = bin/main
SRCS = $(wildcard src/*.c)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f bin/main
