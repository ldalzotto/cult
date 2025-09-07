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
TEST_TARGET=test_runner
COMMON_OBJS=$(OBJ_DIR)/mem.o $(OBJ_DIR)/stack_alloc.o $(OBJ_DIR)/backtrace.o $(OBJ_DIR)/assert.o

all: build/$(TARGET)

# Dependency generation rule
$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	mkdir -p $(DEP_DIR)
	$(CC) $(CFLAGS) -MM $< -MT $(OBJ_DIR)/$*.o -MF $@

$(DEP_DIR)/tests/%.d: tests/%.c
	mkdir -p $(DEP_DIR)/tests
	$(CC) $(CFLAGS) -MM $< -MT $(OBJ_DIR)/tests/$*.o -MF $@ -I src

# Object file compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.c
	mkdir -p $(OBJ_DIR)/tests
	$(CC) $(CFLAGS) -c $< -o $@ -I src

build/$(TARGET): $(OBJ_DIR)/main.o $(COMMON_OBJS)
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) $(OBJ_DIR)/main.o $(COMMON_OBJS)

# Include the dependency files (if they exist)
-include $(DEP_DIR)/main.d
-include $(DEP_DIR)/mem.d
-include $(DEP_DIR)/stack_alloc.d
-include $(DEP_DIR)/backtrace.d

test: build/$(TEST_TARGET)
	./build/$(TEST_TARGET)

build/$(TEST_TARGET): $(OBJ_DIR)/tests/all_tests.o $(OBJ_DIR)/tests/test_mem.o $(OBJ_DIR)/tests/test_stack_alloc.o $(OBJ_DIR)/tests/test_framework.o $(COMMON_OBJS)
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TEST_TARGET) $(OBJ_DIR)/tests/all_tests.o $(OBJ_DIR)/tests/test_mem.o $(OBJ_DIR)/tests/test_stack_alloc.o $(OBJ_DIR)/tests/test_framework.o $(COMMON_OBJS)

# Include test dependency files
-include $(DEP_DIR)/tests/all_tests.d
-include $(DEP_DIR)/tests/test_mem.d
-include $(DEP_DIR)/tests/test_stack_alloc.d
-include $(DEP_DIR)/tests/test_framework.d

clean:
	rm -rf build
	mkdir build
	./generate_compile_commands.sh
