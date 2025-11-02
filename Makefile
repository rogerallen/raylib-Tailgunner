
# Makefile for raylib game on Linux

# Project name
PROJECT_NAME ?= tailgunner

# Directories
SRC_DIR = src
OBJ_DIR = obj

# Compiler and flags
CC = gcc
CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces
CFLAGS += -g -O0 # Debug flags

# Raylib path
RAYLIB_PATH ?= /home/rallen/Documents/Devel/raylib/raylib-5.5_linux_amd64

# Include and library paths
INCLUDE_PATHS = -I$(SRC_DIR) -I$(RAYLIB_PATH)/include
LDFLAGS = -L$(RAYLIB_PATH)/lib
LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Targets
.PHONY: all clean

all: $(PROJECT_NAME)

$(PROJECT_NAME): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(PROJECT_NAME)

run: all
	LD_LIBRARY_PATH=$(RAYLIB_PATH)/lib ./$(PROJECT_NAME)
