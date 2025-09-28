
# returns directory without trailing slash
define get-dir
$(patsubst %/,%,$(dir $1))
endef

# returns base name without extension
define get-basename
$(basename $(notdir $1))
endef

define make_object

$(eval NAME := $1)
$(eval SRC_FILE := $2)
$(eval FLAGS := $3)
$(eval ADDITIONAL_DEPENDENCIES := $4)
$(eval BUILD_DIR := $5)

$(eval OUT_DIR := $(BUILD_DIR)/$(call get-dir,$(SRC_FILE)))
$(eval OUT_BASE_FILE_NO_EXTENSION := $(OUT_DIR)/$(call get-basename,$(SRC_FILE)))
$(eval OBJ_FILE := $(OUT_BASE_FILE_NO_EXTENSION).o)
$(eval DEP_FILE := $(OUT_BASE_FILE_NO_EXTENSION).d)

# build the dependency
$(DEP_FILE): $(SRC_FILE)
	mkdir -p $(OUT_DIR)
	$(CC) $(FLAGS) -MM $(SRC_FILE) -MF $(DEP_FILE) -MT $(DEP_FILE)

# build the object
$(OBJ_FILE): $(SRC_FILE) $(ADDITIONAL_DEPENDENCIES)
	$(CC) $(FLAGS) -c $(SRC_FILE) -o $(OBJ_FILE)

-include $(DEP_FILE)

$(NAME) = $(OBJ_FILE)

endef

define make_executable

$(eval NAME := $1)
$(eval OBJECTS := $2)
$(eval FLAGS := $3)
$(eval BUILD_DIR := $4)

$(eval OUT_DIR := $(BUILD_DIR))
$(eval OUT_BASE_FILE_NO_EXTENSION := $(OUT_DIR)/$(NAME))

$(eval EXECUTABLE_FILE := $(OUT_BASE_FILE_NO_EXTENSION))

$(EXECUTABLE_FILE): $(OBJECTS)
	$(CC) $(OBJECTS) $(FLAGS) -o $(EXECUTABLE_FILE)

$(NAME) = $(EXECUTABLE_FILE)

endef

CC:=gcc
LFLAGS:=-no-pie
SRC_DIR:=src
ELIBS_DIR:=elibs
TESTS_DIR:=tests
BUILD_DIR:=build

# Update CFLAGS with flavor
CFLAGS:=
CFLAGS_SHARED := -Wall -Wextra -Werror -pedantic
FLAVOR ?= DEBUG
ifeq ($(FLAVOR),DEBUG)
    CFLAGS := $(CFLAGS_SHARED) -O0 -g -DDEBUG_ASSERTIONS_ENABLED=1
else ifeq ($(FLAVOR),RELEASE)
    CFLAGS := $(CFLAGS_SHARED) -O3 -DNDEBUG -DDEBUG_ASSERTIONS_ENABLED=0
else
    $(error Unknown FLAVOR '$(FLAVOR)'. Use FLAVOR=DEBUG or FLAVOR=RELEASE)
endif

# Common

COMMON_CFLAGS := -I$(SRC_DIR)
CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS)
$(eval $(call make_object, mem_o, $(SRC_DIR)/mem.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, stack_alloc_o, $(SRC_DIR)/stack_alloc.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, system_time_o, $(SRC_DIR)/system_time.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, time_o, $(SRC_DIR)/time.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, backtrace_o, $(SRC_DIR)/backtrace.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, assert_o, $(SRC_DIR)/assert.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, file_o, $(SRC_DIR)/file.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, print_o, $(SRC_DIR)/print.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, meta_iterator_o, $(SRC_DIR)/meta_iterator.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, format_iterator_o, $(SRC_DIR)/format_iterator.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, convert_o, $(SRC_DIR)/convert.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

common_o = $(mem_o) \
		  $(stack_alloc_o) \
		  $(system_time_o) \
		  $(time_o) \
		  $(backtrace_o) \
		  $(assert_o) \
		  $(file_o) \
		  $(print_o) \
		  $(meta_iterator_o) \
		  $(format_iterator_o) \
		  $(convert_o)

# Window

# Extract X11 headers

X11_EXTRACTED_DIR := $(SRC_DIR)/elibs/X11
X11_MARKER := $(SRC_DIR)/elibs/X11/.x11_extracted

$(X11_MARKER): $(ELIBS_DIR)/x11_headers.tar.gz
	@echo "Extracting $< into $(SRC_DIR)/elibs/..."
	mkdir -p $(SRC_DIR)/elibs
	rm -rf $(X11_EXTRACTED_DIR)
	tar -xzvf $< -C $(SRC_DIR)/elibs
	touch $@

WINDOW_CFLAGS := -I$(ELIBS_DIR)
WINDOW_LFLAGS := -lX11
CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS) $(WINDOW_CFLAGS)

$(eval $(call make_object, win_x11_o, $(SRC_DIR)/window/win_x11.c, $(CURRENT_CFLAGS), $(X11_MARKER), $(BUILD_DIR)))
$(eval $(call make_object, x11_stub_o, $(SRC_DIR)/window/x11_stub.c, $(CURRENT_CFLAGS), $(X11_MARKER), $(BUILD_DIR)))

window_o = $(win_x11_o)


# Coding

CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS)
$(eval $(call make_object, lzss_o, $(SRC_DIR)/coding/lzss.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, lz_match_brute_o, $(SRC_DIR)/coding/lz_match_brute.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, lz_serialize_o, $(SRC_DIR)/coding/lz_serialize.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, lz_deserialize_o, $(SRC_DIR)/coding/lz_deserialize.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, lz_window_o, $(SRC_DIR)/coding/lz_window.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

coding_o = $(lzss_o) \
		  $(lz_match_brute_o) \
		  $(lz_serialize_o) \
		  $(lz_deserialize_o) \
		  $(lz_window_o)

# Main
CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS) $(WINDOW_CFLAGS)
$(eval $(call make_object, main_o, $(SRC_DIR)/main.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

CURRENT_LFLAGS := $(LFLAGS) $(WINDOW_LFLAGS)
$(eval $(call make_executable, main, $(common_o) $(window_o) $(coding_o) $(main_o), $(CURRENT_LFLAGS), $(BUILD_DIR)))
main: $(main)

# Tests

CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS) $(WINDOW_CFLAGS)
$(eval $(call make_object, all_tests_o, $(TESTS_DIR)/all_tests.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_mem_o, $(TESTS_DIR)/test_mem.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_stack_alloc_o, $(TESTS_DIR)/test_stack_alloc.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_win_x11_o, $(TESTS_DIR)/test_win_x11.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_file_o, $(TESTS_DIR)/test_file.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_print_o, $(TESTS_DIR)/test_print.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_temp_dir_o, $(TESTS_DIR)/test_temp_dir.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_framework_o, $(TESTS_DIR)/test_framework.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_backtrace_o, $(TESTS_DIR)/test_backtrace.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_lzss_o, $(TESTS_DIR)/test_lzss.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, test_fps_ticker_o, $(TESTS_DIR)/test_fps_ticker.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

tests_o = $(all_tests_o) \
	 	 $(test_mem_o) \
	 	 $(test_stack_alloc_o) \
	 	 $(test_win_x11_o) \
	 	 $(test_file_o) \
	 	 $(test_print_o) \
	 	 $(test_temp_dir_o) \
	 	 $(test_framework_o) \
	 	 $(test_backtrace_o) \
	 	 $(test_lzss_o) \
	 	 $(test_fps_ticker_o)

CURRENT_LFLAGS := $(LFLAGS) $(WINDOW_LFLAGS)
$(eval $(call make_executable, test, $(common_o) $(window_o) $(coding_o) $(tests_o), $(CURRENT_LFLAGS), $(BUILD_DIR)))
test: $(test)

all: main test

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(SRC_DIR)/elibs/X11
	mkdir $(BUILD_DIR)
	./generate_compile_commands.sh
