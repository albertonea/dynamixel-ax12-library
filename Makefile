CC = gcc
AR = ar
RM = rm -f
MKDIR = mkdir -p

# Project names
PROJECT_NAME = robotarm
LIBNAME = $(PROJECT_NAME)
STATIC_LIB = lib/$(LIBNAME).a
SHARED_LIB = lib/$(LIBNAME).so

# Directories
VENDOR_DIR = vendor
BUILD_DIR = obj
LIB_DIR = lib
BIN_DIR = bin
SRC_DIR = src
TOOLS_DIR = tools

# Flags
CFLAGS = -std=c99 -Wall -Wextra -O2 -Iinclude -Isrc -Itools -I$(VENDOR_DIR)
LDFLAGS =
ARFLAGS = rcs

# Library sources and objects
LIB_SRC = $(wildcard $(SRC_DIR)/*.c)
LIB_OBJ = $(LIB_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
VENDOR_OBJ = $(VENDOR_DIR)/dynamixel/dynamixel.o
ALL_LIB_OBJ = $(LIB_OBJ) $(VENDOR_OBJ)

# Tool-specific sources (EDIT THESE LISTS FOR YOUR TOOLS)
HANOI_SOLVER_SRCS = tools/hanoi_solver.c
EMULATOR_SRCS = tools/emulator.c

# Tool objects
HANOI_SOLVER_OBJ = $(HANOI_SOLVER_SRCS:$(TOOLS_DIR)/%.c=$(BUILD_DIR)/%.o)
EMULATOR_OBJ = $(EMULATOR_SRCS:$(TOOLS_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: help
help:  ## List all tasks with descriptions
	@grep '^[a-zA-Z0-9_-]*:.*?## .*$$' $(MAKEFILE_LIST) | \
	sort | \
	awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

# Default target
.PHONY: all
all: $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(STATIC_LIB) $(BIN_DIR)/hanoi_solver $(BIN_DIR)/emulator

# Directories
$(BUILD_DIR) $(LIB_DIR) $(BIN_DIR):
	$(MKDIR) -p $@

# Library build
$(STATIC_LIB): $(ALL_LIB_OBJ)
	$(MKDIR) -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

# Tool builds (each links only its own objects + lib)
$(BIN_DIR)/hanoi_solver: $(HANOI_SOLVER_OBJ) $(STATIC_LIB)
	$(MKDIR) -p $(dir $@)
	$(CC) $^ -L$(LIB_DIR) -l$(LIBNAME) -o $@ $(LDFLAGS)

$(BIN_DIR)/emulator: $(EMULATOR_OBJ) $(STATIC_LIB)
	$(MKDIR) -p $(dir $@)
	$(CC) $^ -L$(LIB_DIR) -l$(LIBNAME) -o $@ $(LDFLAGS)

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

# Install (public headers + lib + tools)
PREFIX ?= /usr/local
.PHONY: install
install: $(STATIC_LIB) $(BIN_DIR)/hanoi_solver $(BIN_DIR)/emulator
	install -d $(PREFIX)/lib
	install $(STATIC_LIB) $(PREFIX)/lib/
	install -d $(PREFIX)/include/$(PROJECT_NAME)
	install include/*.h $(PREFIX)/include/$(PROJECT_NAME)/
	install -d $(PREFIX)/bin
	install $(BIN_DIR)/* $(PREFIX)/bin/
