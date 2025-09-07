SRC_DIR=src
DEP_DIR=build/deps
OBJ_DIR=build/objects
CC=gcc
CFLAGS=-Wall -Wextra -g -DDEBUG_ASSERTIONS_ENABLED=1
TARGET=myapp

all: build/$(TARGET)

# Dependency generation rule
$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	mkdir -p $(DEP_DIR)
	$(CC) $(CFLAGS) -MM $< -MT $(OBJ_DIR)/$*.o -MF $@

# Object file compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

build/$(TARGET): $(OBJ_DIR)/main.o $(OBJ_DIR)/mem.o
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) $(OBJ_DIR)/main.o $(OBJ_DIR)/mem.o

# Include the dependency files (if they exist)
-include $(DEP_DIR)/main.d
-include $(DEP_DIR)/mem.d

clean:
	rm -rf build
