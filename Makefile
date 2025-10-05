
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
TOOLS_DIR:=tools
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

COMMON_CFLAGS := -I$(SRC_DIR)/libs
CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS)
$(eval $(call make_object, mem_o, $(SRC_DIR)/libs/mem.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, stack_alloc_o, $(SRC_DIR)/libs/stack_alloc.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, system_time_o, $(SRC_DIR)/libs/system_time.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, time_o, $(SRC_DIR)/libs/time.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, backtrace_o, $(SRC_DIR)/libs/backtrace.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, assert_o, $(SRC_DIR)/libs/assert.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, file_o, $(SRC_DIR)/libs/file.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, print_o, $(SRC_DIR)/libs/print.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, meta_iterator_o, $(SRC_DIR)/libs/meta_iterator.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, format_iterator_o, $(SRC_DIR)/libs/format_iterator.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, convert_o, $(SRC_DIR)/libs/convert.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

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

# We may want to cache this
USE_X11 := $(shell pkg-config --exists x11 && echo 1 || echo 0)

X11_EXTRACTED_DIR := $(SRC_DIR)/libs/elibs/X11
X11_MARKER := $(SRC_DIR)/libs/elibs/X11/.x11_extracted

$(X11_MARKER): $(ELIBS_DIR)/x11_headers.tar.gz
	@echo "Extracting $< into $(SRC_DIR)/libs/elibs/..."
	mkdir -p $(SRC_DIR)/libs/elibs
	rm -rf $(X11_EXTRACTED_DIR)
	tar -xzvf $< -C $(SRC_DIR)/libs/elibs
	touch $@

WINDOW_CFLAGS := -I$(SRC_DIR)/libs/elibs
WINDOW_LFLAGS := 
ifeq ($(USE_X11), 1)
	WINDOW_LFLAGS += -lX11
endif
CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS) $(WINDOW_CFLAGS)

$(eval $(call make_object, win_x11_o, $(SRC_DIR)/libs/window/win_x11.c, $(CURRENT_CFLAGS), $(X11_MARKER), $(BUILD_DIR)))
$(eval $(call make_object, x11_stub_o, $(SRC_DIR)/libs/window/x11_stub.c, $(CURRENT_CFLAGS), $(X11_MARKER), $(BUILD_DIR)))

window_o = $(win_x11_o)
ifeq ($(USE_X11), 0)
	window_o += $(x11_stub_o)
endif

# Coding

CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS)
$(eval $(call make_object, lzss_o, $(SRC_DIR)/libs/coding/lzss.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, lz_match_brute_o, $(SRC_DIR)/libs/coding/lz_match_brute.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, lz_serialize_o, $(SRC_DIR)/libs/coding/lz_serialize.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, lz_deserialize_o, $(SRC_DIR)/libs/coding/lz_deserialize.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, lz_window_o, $(SRC_DIR)/libs/coding/lz_window.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

coding_o = $(lzss_o) \
		  $(lz_match_brute_o) \
		  $(lz_serialize_o) \
		  $(lz_deserialize_o) \
		  $(lz_window_o)

# Network

CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS)
$(eval $(call make_object, tcp_connection_o, $(SRC_DIR)/libs/network/tcp/tcp_connection.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, https_request_o, $(SRC_DIR)/libs/network/https/https_request.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

network_o = $(https_request_o) \
			$(tcp_connection_o)

NETWORK_LFLAGS := -lssl

# Dummy

CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS) $(WINDOW_CFLAGS)
$(eval $(call make_object, dummy_o, $(SRC_DIR)/apps/dummy/dummy.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

CURRENT_LFLAGS := $(LFLAGS) $(WINDOW_LFLAGS)
$(eval $(call make_executable, dummy, $(common_o) $(window_o) $(coding_o) $(dummy_o), $(CURRENT_LFLAGS), $(BUILD_DIR)))
dummy: $(dummy)

# Snake
CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS) $(WINDOW_CFLAGS)
$(eval $(call make_object, snake_o, $(SRC_DIR)/apps/snake/snake.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, snake_loop_o, $(SRC_DIR)/apps/snake/snake_loop.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, snake_grid_o, $(SRC_DIR)/apps/snake/snake_grid.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, snake_move_o, $(SRC_DIR)/apps/snake/snake_move.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, snake_reward_o, $(SRC_DIR)/apps/snake/snake_reward.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

snake_module_o = $(snake_o)\
		 $(snake_grid_o) \
		 $(snake_move_o) \
		 $(snake_reward_o)

snake_executable_o = $(snake_loop_o)

CURRENT_LFLAGS := $(LFLAGS) $(WINDOW_LFLAGS)
$(eval $(call make_executable, snake, $(common_o) $(window_o) $(coding_o) $(snake_module_o) $(snake_executable_o), $(CURRENT_LFLAGS), $(BUILD_DIR)))
snake: $(snake)

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
$(eval $(call make_object, test_snake_o, $(TESTS_DIR)/test_snake.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))

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
	 	 $(test_fps_ticker_o) \
		 $(test_snake_o)

CURRENT_LFLAGS := $(LFLAGS) $(WINDOW_LFLAGS)
$(eval $(call make_executable, test, $(common_o) $(window_o) $(coding_o) $(snake_module_o) $(tests_o), $(CURRENT_LFLAGS), $(BUILD_DIR)))
test: $(test)

# Tools agent
CURRENT_CFLAGS := $(CFLAGS) $(COMMON_CFLAGS)
$(eval $(call make_object, agent_o, $(TOOLS_DIR)/agent/agent.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, user_content_read_o, $(TOOLS_DIR)/agent/user_content_read.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, agent_request_o, $(TOOLS_DIR)/agent/agent_request.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
$(eval $(call make_object, agent_result_write_o, $(TOOLS_DIR)/agent/agent_result_write.c, $(CURRENT_CFLAGS), , $(BUILD_DIR)))
all_agent_o = $(agent_o) \
			$(user_content_read_o) \
			$(agent_request_o) \
			$(agent_result_write_o)

CURRENT_LFLAGS := $(LFLAGS) $(NETWORK_LFLAGS)
$(eval $(call make_executable, agent, $(common_o) $(network_o) $(all_agent_o), $(CURRENT_LFLAGS), $(BUILD_DIR)))
agent: $(agent)

all: snake dummy test agent

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(SRC_DIR)/libs/elibs/X11
	mkdir $(BUILD_DIR)
	./generate_compile_commands.sh
