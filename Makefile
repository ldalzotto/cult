SRC_DIR=src
ELIBS_DIR=elibs
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
COMMON_OBJS=$(OBJ_DIR)/mem.o $(OBJ_DIR)/stack_alloc.o $(OBJ_DIR)/backtrace.o $(OBJ_DIR)/assert.o $(OBJ_DIR)/window/win_x11.o $(OBJ_DIR)/window/win_headless.o
TEST_OBJS=$(OBJ_DIR)/tests/all_tests.o $(OBJ_DIR)/tests/test_mem.o $(OBJ_DIR)/tests/test_stack_alloc.o $(OBJ_DIR)/tests/test_win_x11.o $(OBJ_DIR)/tests/test_win_headless.o $(OBJ_DIR)/tests/test_framework.o
MAIN_DEPS := $(patsubst $(OBJ_DIR)/%.o, $(DEP_DIR)/%.d, $(OBJ_DIR)/main.o $(COMMON_OBJS))
TEST_DEPS := $(patsubst $(OBJ_DIR)/tests/%.o, $(DEP_DIR)/tests/%.d, $(TEST_OBJS)) $(patsubst $(OBJ_DIR)/%.o, $(DEP_DIR)/%.d, $(COMMON_OBJS))

X11_EXTRACTED_DIR = $(SRC_DIR)/elibs/X11
X11_MARKER = $(SRC_DIR)/elibs/X11/.x11_extracted

$(X11_MARKER): $(ELIBS_DIR)/x11_headers.tar.gz
	@if [ ! -d "$(X11_EXTRACTED_DIR)" ]; then \
	    echo "Extracting $< into $(SRC_DIR)/elibs/..."; \
	    rm -rf $(X11_EXTRACTED_DIR); \
	    tar -xzvf $< -C $(SRC_DIR)/elibs; \
	fi
	touch $@

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

$(OBJ_DIR)/window/win_x11.o: $(SRC_DIR)/window/win_x11.c $(X11_MARKER)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR)/elibs -c $< -o $@

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

build/$(TEST_TARGET): $(TEST_OBJS) $(COMMON_OBJS)
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TEST_TARGET) $(TEST_OBJS) $(COMMON_OBJS) $(LDFLAGS)

# Include test dependency files
-include $(TEST_DEPS)

all: build/$(TARGET) build/$(TEST_TARGET)

clean:
	rm -rf build
	rm -rf $(SRC_DIR)/elibs/X11
	mkdir build
	./generate_compile_commands.sh
