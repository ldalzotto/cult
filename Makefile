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
LDFLAGS=-lX11
COMMON_OBJS=$(OBJ_DIR)/mem.o $(OBJ_DIR)/stack_alloc.o $(OBJ_DIR)/backtrace.o $(OBJ_DIR)/assert.o $(OBJ_DIR)/window/win_x11.o
MAIN_DEPS := $(patsubst $(OBJ_DIR)/%.o, $(DEP_DIR)/%.d, $(OBJ_DIR)/main.o $(COMMON_OBJS))
TEST_DEPS := $(patsubst $(OBJ_DIR)/tests/%.o, $(DEP_DIR)/tests/%.d, $(OBJ_DIR)/tests/all_tests.o $(OBJ_DIR)/tests/test_mem.o $(OBJ_DIR)/tests/test_stack_alloc.o $(OBJ_DIR)/tests/test_framework.o) $(patsubst $(OBJ_DIR)/%.o, $(DEP_DIR)/%.d, $(COMMON_OBJS))

# Dependency generation rule
$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MM $< -MT $(OBJ_DIR)/$*.o -MF $@

$(DEP_DIR)/tests/%.d: tests/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MM $< -MT $(OBJ_DIR)/tests/$*.o -MF $@ -I src

# Object file compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.c
	mkdir -p $(OBJ_DIR)/tests
	$(CC) $(CFLAGS) -c $< -o $@ -I src

build/$(TARGET): $(OBJ_DIR)/main.o $(COMMON_OBJS)
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) $(OBJ_DIR)/main.o $(COMMON_OBJS) $(LDFLAGS)

# Include the dependency files (if they exist)
-include $(MAIN_DEPS)

test: build/$(TEST_TARGET)
	./build/$(TEST_TARGET)

build/$(TEST_TARGET): $(OBJ_DIR)/tests/all_tests.o $(OBJ_DIR)/tests/test_mem.o $(OBJ_DIR)/tests/test_stack_alloc.o $(OBJ_DIR)/tests/test_framework.o $(COMMON_OBJS)
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TEST_TARGET) $(OBJ_DIR)/tests/all_tests.o $(OBJ_DIR)/tests/test_mem.o $(OBJ_DIR)/tests/test_stack_alloc.o $(OBJ_DIR)/tests/test_framework.o $(COMMON_OBJS) $(LDFLAGS)

# Include test dependency files
-include $(TEST_DEPS)

all: build/$(TARGET) build/$(TEST_TARGET)

clean:
	rm -rf build
	mkdir build
	./generate_compile_commands.sh
