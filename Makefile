SRC_DIR=src
DEP_DIR=build/deps
OBJ_DIR=build/objects
CC=gcc
CFLAGS=-Wall -Wextra -g
TARGET=myapp

all: build/$(TARGET)

# Dependency generation rule
$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	mkdir -p $(DEP_DIR)
	$(CC) -MM $< -MT $(OBJ_DIR)/$*.o -MF $@

# Object file compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

build/$(TARGET): $(OBJ_DIR)/main.o
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) $(OBJ_DIR)/main.o

# Include the dependency files (if they exist)
-include $(DEP_DIR)/main.d

clean:
	rm -rf build
