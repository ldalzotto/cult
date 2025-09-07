SRC_DIR=src
DEP_DIR=build/deps
OBJ_DIR=build/objects
CC=gcc
CFLAGS_SHARED=-Wall -Wextra -pedantic

# Default build flavor
FLAVOR ?= DEBUG

# Flags per flavor
ifeq ($(FLAVOR),DEBUG)
    CFLAGS=$(CFLAGS_SHARED) -O0 -g -no-pie -DDEBUG_ASSERTIONS_ENABLED=1
else ifeq ($(FLAVOR),RELEASE)
    CFLAGS=$(CFLAGS_SHARED) -O3 -DNDEBUG -DDEBUG_ASSERTIONS_ENABLED=0
else
    $(error Unknown FLAVOR '$(FLAVOR)'. Use FLAVOR=DEBUG or FLAVOR=RELEASE)
endif

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

build/$(TARGET): $(OBJ_DIR)/main.o $(OBJ_DIR)/mem.o $(OBJ_DIR)/stack_alloc.o $(OBJ_DIR)/backtrace.o
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) $(OBJ_DIR)/main.o $(OBJ_DIR)/mem.o $(OBJ_DIR)/stack_alloc.o $(OBJ_DIR)/backtrace.o

# Include the dependency files (if they exist)
-include $(DEP_DIR)/main.d
-include $(DEP_DIR)/mem.d
-include $(DEP_DIR)/stack_alloc.d
-include $(DEP_DIR)/backtrace.d

clean:
	rm -rf build
	mkdir build

test: build/test_runner
	./build/test_runner

# TODO: use the module for compile_commands.json and header rebuild
build/test_runner: tests/all_tests.c tests/test_mem.c tests/test_stack_alloc.c src/mem.c src/stack_alloc.c src/backtrace.c
	mkdir -p build
	$(CC) $(CFLAGS) -o build/test_runner tests/all_tests.c tests/test_mem.c tests/test_stack_alloc.c src/mem.c src/stack_alloc.c src/backtrace.c -I src
