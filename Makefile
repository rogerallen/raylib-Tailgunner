# Makefile for raylib game on Linux and Web

# Project name
PROJECT_NAME ?= tailgunner

# Directories
SRC_DIR = src

# Default platform
PLATFORM ?= native

# Default object directory
OBJ_DIR = obj/$(PLATFORM)

# Native build raylib path where lib/libraylib.* files are located
# Download from https://github.com/raysan5/raylib/releases
RAYLIB_NATIVE_PATH ?= /home/rallen/Documents/Devel/raylib/raylib-5.5_linux_amd64

# Emscripten environment
EMSDK_PATH ?= /home/rallen/Documents/Devel/emscripten/emsdk
EMSDK_ENV = $(EMSDK_PATH)/emsdk_env.sh
# Path to where raylib/libraylib.a is located.  Typically you will need to build this yourself.
RAYLIB_EMSCRIPTEN_PATH ?= /home/rallen/Documents/Devel/raylib/raylib

# Build type
DEBUG ?= 0

# Compiler and flags
ifeq ($(PLATFORM), web)
    CC = emcc
    CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -DPLATFORM_WEB -MMD -MP
    ifeq ($(DEBUG), 1)
        LDFLAGS = -O0 -g -s ASSERTIONS=1 
    else
        LDFLAGS = -O3 -s ASSERTIONS=0
    endif
    LDFLAGS += -s USE_GLFW=3 -s ASYNCIFY --preload-file resources
    RAYLIB_PATH = $(RAYLIB_EMSCRIPTEN_PATH)
    INCLUDE_PATHS = -I$(SRC_DIR) -I$(RAYLIB_PATH)/src
    LIBRARY_PATHS = -L$(RAYLIB_PATH)/raylib
    LDLIBS = -lraylib
    TARGET = $(PROJECT_NAME).html
else
    CC = gcc
    CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -MMD -MP
    ifeq ($(DEBUG), 1)
        CFLAGS += -g -O0 # Debug flags
    else
        CFLAGS += -O2 # Release flags
    endif
    RAYLIB_PATH = $(RAYLIB_NATIVE_PATH)
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
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LDLIBS) $(LDFLAGS) --shell-file shell.html
else
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf obj $(PROJECT_NAME) $(PROJECT_NAME).data $(PROJECT_NAME).html $(PROJECT_NAME).js $(PROJECT_NAME).wasm

run: all
	LD_LIBRARY_PATH=$(RAYLIB_PATH)/lib ./$(PROJECT_NAME)

webserve:
	python3 -m http.server 8000

-include $(OBJS:.o=.d)
