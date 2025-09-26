SRC_DIR := src
ELIBS_DIR := elibs
TESTS_DIR := tests
DEP_DIR := build/deps
OBJ_DIR := build/objects
CC := gcc
CFLAGS_SHARED := -Wall -Wextra -Werror -pedantic
LDFLAGS :=

FLAVOR ?= DEBUG

ifeq ($(FLAVOR),DEBUG)
    CFLAGS := $(CFLAGS_SHARED) -O0 -g -no-pie -DDEBUG_ASSERTIONS_ENABLED=1
else ifeq ($(FLAVOR),RELEASE)
    CFLAGS := $(CFLAGS_SHARED) -O3 -DNDEBUG -DDEBUG_ASSERTIONS_ENABLED=0
else
    $(error Unknown FLAVOR '$(FLAVOR)'. Use FLAVOR=DEBUG or FLAVOR=RELEASE)
endif

TARGET := myapp
TEST_TARGET := test_runner

# ----------------------
# Module definitions
# ----------------------

# Common module
COMMON_MODULE_OBJS := \
    $(OBJ_DIR)/mem.o \
    $(OBJ_DIR)/stack_alloc.o \
    $(OBJ_DIR)/system_time.o \
    $(OBJ_DIR)/time.o \
    $(OBJ_DIR)/backtrace.o \
    $(OBJ_DIR)/assert.o \
    $(OBJ_DIR)/file.o \
    $(OBJ_DIR)/print.o \
    $(OBJ_DIR)/meta_iterator.o \
    $(OBJ_DIR)/format_iterator.o \
    $(OBJ_DIR)/convert.o \
    $(OBJ_DIR)/window/win_x11.o \
    $(OBJ_DIR)/coding/lzss.o \
    $(OBJ_DIR)/coding/lz_match_brute.o \
    $(OBJ_DIR)/coding/lz_serialize.o \
    $(OBJ_DIR)/coding/lz_deserialize.o \
    $(OBJ_DIR)/coding/lz_window.o

# Auto-generate dependency files for common module
COMMON_DEPS := $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$(COMMON_MODULE_OBJS))

common_MODULE: $(COMMON_MODULE_OBJS) $(COMMON_DEPS)
	@echo "Common module built."

# Test module
TEST_MODULE_OBJS := \
    $(OBJ_DIR)/$(TESTS_DIR)/all_tests.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_mem.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_stack_alloc.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_win_x11.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_file.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_print.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_temp_dir.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_framework.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_backtrace.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_lzss.o \
    $(OBJ_DIR)/$(TESTS_DIR)/test_fps_ticker.o

test_MODULE: common_MODULE $(TEST_MODULE_OBJS)
	@echo "Test module built."

# ----------------------
# X11 detection
# ----------------------
USE_X11 := $(shell pkg-config --exists x11 && echo 1 || echo 0)

ifeq ($(USE_X11), 1)
	LDFLAGS += -lX11
else
	COMMON_MODULE_OBJS += $(OBJ_DIR)/window/x11_stub.o
endif

X11_EXTRACTED_DIR := $(SRC_DIR)/elibs/X11
X11_MARKER := $(SRC_DIR)/elibs/X11/.x11_extracted

$(X11_MARKER): $(ELIBS_DIR)/x11_headers.tar.gz
	@echo "Extracting $< into $(SRC_DIR)/elibs/..."
	mkdir -p $(SRC_DIR)/elibs
	rm -rf $(X11_EXTRACTED_DIR)
	tar -xzvf $< -C $(SRC_DIR)/elibs
	touch $@

# ----------------------
# Dependency generation
# ----------------------
MAIN_DEPS := $(DEP_DIR)/main.d $(COMMON_DEPS)
TEST_DEPS := $(patsubst $(OBJ_DIR)/$(TESTS_DIR)/%.o, $(DEP_DIR)/$(TESTS_DIR)/%.d, $(TEST_MODULE_OBJS)) \
             $(COMMON_DEPS)

$(DEP_DIR)/%.d: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I $(SRC_DIR) -MM $< -MT $(OBJ_DIR)/$*.o -MF $@

$(DEP_DIR)/$(TESTS_DIR)/%.d: $(TESTS_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I $(SRC_DIR) -I $(TESTS_DIR) -MM $< -MT $(OBJ_DIR)/$(TESTS_DIR)/$*.o -MF $@

# ----------------------
# Object compilation
# ----------------------
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

# ----------------------
# Targets
# ----------------------
build/$(TARGET): $(OBJ_DIR)/main.o common_MODULE
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)/main.o $(COMMON_MODULE_OBJS) $(LDFLAGS)

build/$(TEST_TARGET): test_MODULE
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(TEST_MODULE_OBJS) $(COMMON_MODULE_OBJS) $(LDFLAGS)

# Include dependencies
-include $(MAIN_DEPS)
-include $(TEST_DEPS)

all: build/$(TARGET) build/$(TEST_TARGET)

myapp: build/$(TARGET)
	./build/$(TARGET)

test: build/$(TEST_TARGET)
	./build/$(TEST_TARGET)

clean:
	rm -rf build
	rm -rf $(SRC_DIR)/elibs/X11
	mkdir build
	./generate_compile_commands.sh
