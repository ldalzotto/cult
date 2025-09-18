SRC_DIR=src
ELIBS_DIR=elibs
TESTS_DIR=tests
DEP_DIR=build/deps
OBJ_DIR=build/objects
CC=gcc
CFLAGS_SHARED=-Wall -Wextra -Werror -pedantic
LDFLAGS=

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
COMMON_OBJS= \
	$(OBJ_DIR)/mem.o \
	$(OBJ_DIR)/stack_alloc.o \
	$(OBJ_DIR)/backtrace.o \
	$(OBJ_DIR)/assert.o \
	$(OBJ_DIR)/file.o \
	$(OBJ_DIR)/print.o \
	$(OBJ_DIR)/meta_iterator.o \
	$(OBJ_DIR)/format_iterator.o \
	$(OBJ_DIR)/convert.o \
	$(OBJ_DIR)/window/win_x11.o

TEST_OBJS= \
	$(OBJ_DIR)/$(TESTS_DIR)/all_tests.o \
	$(OBJ_DIR)/$(TESTS_DIR)/test_mem.o \
	$(OBJ_DIR)/$(TESTS_DIR)/test_stack_alloc.o \
	$(OBJ_DIR)/$(TESTS_DIR)/test_win_x11.o \
	$(OBJ_DIR)/$(TESTS_DIR)/test_file.o \
	$(OBJ_DIR)/$(TESTS_DIR)/test_print.o \
	$(OBJ_DIR)/$(TESTS_DIR)/test_temp_dir.o \
	$(OBJ_DIR)/$(TESTS_DIR)/test_framework.o

# We may want to cache this
USE_X11 := $(shell pkg-config --exists x11 && echo 1 || echo 0)

ifeq ($(USE_X11), 1)
	LDFLAGS += -lX11
else
	COMMON_OBJS += $(OBJ_DIR)/window/x11_stub.o
endif

X11_EXTRACTED_DIR = $(SRC_DIR)/elibs/X11
X11_MARKER = $(SRC_DIR)/elibs/X11/.x11_extracted

$(X11_MARKER): $(ELIBS_DIR)/x11_headers.tar.gz
	echo "Extracting $< into $(SRC_DIR)/elibs/..."
	mkdir -p $(SRC_DIR)/elibs
	rm -rf $(X11_EXTRACTED_DIR)
	tar -xzvf $< -C $(SRC_DIR)/elibs
	touch $@

MAIN_DEPS := $(patsubst $(OBJ_DIR)/%.o, $(DEP_DIR)/%.d, $(OBJ_DIR)/main.o $(COMMON_OBJS))
TEST_DEPS := $(patsubst $(OBJ_DIR)/$(TESTS_DIR)/%.o, $(DEP_DIR)/$(TESTS_DIR)/%.d, $(TEST_OBJS)) $(patsubst $(OBJ_DIR)/%.o, $(DEP_DIR)/%.d, $(COMMON_OBJS))


# Dependency generation rule
$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I $(SRC_DIR) -MM $< -MT $(OBJ_DIR)/$*.o -MF $@

$(DEP_DIR)/$(TESTS_DIR)/%.d: $(TESTS_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I $(SRC_DIR) -I $(TESTS_DIR) -MM $< -MT $(OBJ_DIR)/$(TESTS_DIR)/$*.o -MF $@

# Object file compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I $(SRC_DIR) -c $< -o $@

$(OBJ_DIR)/window/win_x11.o: $(SRC_DIR)/window/win_x11.c $(X11_MARKER)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I $(SRC_DIR) -I$(SRC_DIR)/elibs -c $< -o $@

$(OBJ_DIR)/window/x11_stub.o: $(SRC_DIR)/window/x11_stub.c $(X11_MARKER)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I $(SRC_DIR) -I$(SRC_DIR)/elibs -c $< -o $@

$(OBJ_DIR)/$(TESTS_DIR)/%.o: $(TESTS_DIR)/%.c
	mkdir -p $(OBJ_DIR)/$(TESTS_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ -I src -I $(TESTS_DIR)

build/$(TARGET): $(OBJ_DIR)/main.o $(COMMON_OBJS)
	mkdir -p build
	$(CC) $(CFLAGS) -o build/$(TARGET) $(OBJ_DIR)/main.o $(COMMON_OBJS) $(LDFLAGS)

# Include the dependency files (if they exist)
-include $(MAIN_DEPS)

myapp: build/$(TARGET)
	   ./build/$(TARGET)

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
