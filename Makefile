# Compiler
CC = gcc

# Directories
SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build
LOGS_DIR = logs

# Target executable
TARGET = flight_computer_dump

# Compiler flags
CFLAGS = -Wall -Wextra -I$(INC_DIR)

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Link object files into executable in root directory
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)
	rm -rf $(LOGS_DIR)/*

# Purge log files
purge: 
	rm -rf $(LOGS_DIR)/*

# Rebuild everything
rebuild: clean all

.PHONY: all clean rebuild
