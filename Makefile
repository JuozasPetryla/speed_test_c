TARGET_EXEC := main

CC := gcc
CFLAGS := -Wall -Wextra
CPPFLAGS := -Isrc -Ilib

BUILD_DIR := build
SRC_DIR := src
LIB_DIR := lib

SRC     := $(wildcard $(SRC_DIR)/*.c)
LIB_SRC := $(wildcard $(LIB_DIR)/*/*.c)

SRC_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/src/%.o,$(SRC))
LIB_OBJS := $(patsubst $(LIB_DIR)/%/%.c,$(BUILD_DIR)/lib/%/%.o,$(LIB_SRC))
OBJS := $(SRC_OBJS) $(LIB_OBJS)

.PHONY: all clean
all: $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(sort $(dir $(OBJS)))

$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/%/%.o: $(LIB_DIR)/%/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@

clean:
	rm -rf $(BUILD_DIR)
