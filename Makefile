SRC_DIR=src
CC=gcc
CFLAGS=-Wall -Wextra -g
TARGET=myapp

all: build/$(TARGET)

build/$(TARGET): $(SRC_DIR)/main.c
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) $(SRC_DIR)/main.c

clean:
	rm -rf build
