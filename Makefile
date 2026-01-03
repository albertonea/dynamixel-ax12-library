CC = gcc
AR = ar
RM = rm -f
MKDIR = mkdir -p

# Flags
CFLAGS = -std=c99 -Wall -Wextra -O2 -Iinclude -Isrc -Itools -I$(VENDOR_DIR)
LDFLAGS =
ARFLAGS = rcs

# Project names
PROJECT_NAME = robotarm
LIBNAME = $(PROJECT_NAME)
STATIC_LIB = lib/$(LIBNAME).a
SHARED_LIB = lib/$(LIBNAME).so
TOOLS = hanoi_solver emulator  # Replace with your tool names

# Directories
VENDOR_DIR = vendor/dynamixel
BUILD_DIR = obj
LIB_DIR = lib
BIN_DIR = bin
SRC_DIR = src
TOOLS_DIR = tools

# Sources and objects
LIB_SRC = $(wildcard $(SRC_DIR)/*.c)
LIB_OBJ = $(LIB_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TOOL_SRC = $(wildcard $(TOOLS_DIR)/*.c)
TOOL_OBJ = $(TOOL_SRC:$(TOOLS_DIR)/%.c=$(BUILD_DIR)/%.o)
VENDOR_OBJ = $(VENDOR_DIR)/dynamixel.o

# All objects for library (your code + vendor)
ALL_LIB_OBJ = $(LIB_OBJ) $(VENDOR_OBJ)

# Targets for tools
define tool_template
  $(BIN_DIR)/$(1): $(TOOL_OBJ) $(filter-out $(BUILD_DIR)/$(TOOLS_DIR)/$($(1)_MAIN_OBJ),$(TOOL_OBJ)) $(STATIC_LIB)
	$$(MKDIR) -p $$(dir $$@)
	$$(CC) $$^ -L$$(LIB_DIR) -l$(LIBNAME) -o $$@ $$(LDFLAGS)
endef
$(foreach tool,$(TOOLS),$(eval $(call tool_template,$(tool))))

# Default target
.PHONY: all
all: $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(STATIC_LIB) $(addprefix $(BIN_DIR)/,$(TOOLS))

# Directories
$(BUILD_DIR) $(LIB_DIR) $(BIN_DIR):
	$(MKDIR) -p $@

# Library build
$(STATIC_LIB): $(ALL_LIB_OBJ)
	$(MKDIR) -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

# Object compilation rules
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TOOLS_DIR)/%.c
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Shared library (optional)
.PHONY: shared
shared: CFLAGS += -fPIC
shared: LDFLAGS += -shared
shared: $(SHARED_LIB)

$(SHARED_LIB): $(ALL_LIB_OBJ)
	$(CC) $(LDFLAGS) -shared $^ -o $@

# Clean
.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR)

# Install (headers only, no vendor/private)
PREFIX ?= /usr/local
.PHONY: install
install: $(STATIC_LIB)
	install -d $(PREFIX)/lib
	install $(STATIC_LIB) $(PREFIX)/lib/
	install -d $(PREFIX)/include/$(PROJECT_NAME)
	install include/*.h $(PREFIX)/include/$(PROJECT_NAME)/
	install -d $(PREFIX)/bin
	install $(BIN_DIR)/* $(PREFIX)/bin/
