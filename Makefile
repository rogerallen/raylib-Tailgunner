
# Makefile for raylib game on Linux and Web

# Project name
PROJECT_NAME ?= tailgunner

# Directories
SRC_DIR = src

# Default platform
PLATFORM ?= native

# Default object directory
OBJ_DIR = obj/$(PLATFORM)

# Emscripten environment
EMSDK_ENV ?= /home/rallen/Documents/Devel/emscripten/emsdk/emsdk_env.sh

# Compiler and flags
ifeq ($(PLATFORM), web)
    CC = emcc
    CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -DPLATFORM_WEB
    LDFLAGS = -O1 -s USE_GLFW=3 -s ASYNCIFY
    RAYLIB_PATH ?= /home/rallen/Documents/Devel/raylib
    INCLUDE_PATHS = -I$(SRC_DIR) -I$(RAYLIB_PATH)/raylib/src
    LIBRARY_PATHS = -L$(RAYLIB_PATH)/raylib/raylib
    LDLIBS = -lraylib
    TARGET = $(PROJECT_NAME).html
else
    CC = gcc
    CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces
    CFLAGS += -g -O0 # Debug flags
    RAYLIB_PATH ?= /home/rallen/Documents/Devel/raylib/raylib-5.5_linux_amd64
    INCLUDE_PATHS = -I$(SRC_DIR) -I$(RAYLIB_PATH)/include
    LDFLAGS = -L$(RAYLIB_PATH)/lib
    LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    TARGET = $(PROJECT_NAME)
endif

# Files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Targets
.PHONY: all clean web

all: $(TARGET)

web: 
	@bash -c 'source $(EMSDK_ENV) && $(MAKE) all PLATFORM=web'

$(TARGET): $(OBJS)
ifeq ($(PLATFORM), web)
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LDLIBS) $(LDFLAGS) --shell-file $(RAYLIB_PATH)/raylib/src/shell.html
else
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf obj $(PROJECT_NAME) $(PROJECT_NAME).html $(PROJECT_NAME).js $(PROJECT_NAME).wasm

run: all
	LD_LIBRARY_PATH=$(RAYLIB_PATH)/lib ./$(PROJECT_NAME)

webserve:
	python3 -m http.server 8000
