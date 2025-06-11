# Makefile for compiling dragonshell

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -std=c99

# Target executable
TARGET = vishell

# Source files
SRCS = vishell.c

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean up build files
clean:
	rm -f $(TARGET)
