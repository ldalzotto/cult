CC=gcc
CFLAGS=-Wall -Wextra -g
TARGET=myapp

all: build/$(TARGET)

build/$(TARGET): main.c
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) main.c

clean:
	rm -rf build
