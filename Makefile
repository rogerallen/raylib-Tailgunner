# Makefile for raylib game on Linux and Web

# Project name
PROJECT_NAME ?= tailgunner

# Zip number
ZIP_NUMBER ?= 11

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
DEBUG_OPENGL ?= 0

# By default enable extra warnings
EXTRA_WARNINGS = 1

# Compiler and flags
ifeq ($(PLATFORM), web)
    CC = emcc
    CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -DPLATFORM_WEB -MMD -MP
    ifeq ($(DEBUG), 1)
        LDFLAGS = -O0 -g -s ASSERTIONS=1 
    else
        LDFLAGS = -O3 -s ASSERTIONS=0
    endif
    ifeq ($(DEBUG_OPENGL), 1)
        CFLAGS += -DDEBUG_OPENGL
    endif
    LDFLAGS += -s USE_GLFW=3 -s ASYNCIFY --preload-file resources
    RAYLIB_PATH = $(RAYLIB_EMSCRIPTEN_PATH)
    INCLUDE_PATHS = -I$(SRC_DIR) -I$(RAYLIB_PATH)/src
    LIBRARY_PATHS = -L$(RAYLIB_PATH)/raylib
    LDLIBS = -lraylib -lhtml5 -lfetch
    TARGET = $(PROJECT_NAME).html
else
    CC = gcc
    CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -DPLATFORM_DESKTOP -MMD -MP
    ifeq ($(DEBUG), 1)
        CFLAGS += -g -O0 # Debug flags
    else
        CFLAGS += -O2 # Release flags
    endif
    ifeq ($(DEBUG_OPENGL), 1)
        CFLAGS += -DDEBUG_OPENGL
    endif
    ifeq ($(ASAN), 1)
        CFLAGS += -fsanitize=address -fno-omit-frame-pointer -g
        LDFLAGS += -fsanitize=address
    endif
    ifeq ($(EXTRA_WARNINGS), 1)
        # CFLAGS += -Wextra -Wpedantic -Wstrict-prototypes -Wwrite-strings -Wconversion
        # conversions was just too much
        CFLAGS += -Wextra -Wpedantic -Wstrict-prototypes -Wwrite-strings
    endif
    RAYLIB_PATH = $(RAYLIB_NATIVE_PATH)
    INCLUDE_PATHS = -I$(SRC_DIR) -I$(RAYLIB_PATH)/include
    LDFLAGS = -L$(RAYLIB_PATH)/lib
    LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lcurl
    TARGET = $(PROJECT_NAME)
endif

# Files
SRC = $(wildcard $(SRC_DIR)/*.c) $(SRC_DIR)/cJSON.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Targets
.PHONY: all clean web analyze asan valgrind cppcheck clang-tidy scan-build gcc-warnings

all: $(TARGET)

web: 
	@bash -c 'source $(EMSDK_ENV) && $(MAKE) all PLATFORM=web'

$(TARGET): $(OBJS)
ifeq ($(PLATFORM), web)
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LDLIBS) $(LDFLAGS) --shell-file shell.html
	cp $(PROJECT_NAME).html index.html
else
	$(CC) -o $@ $^ $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)
endif

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf obj $(PROJECT_NAME) $(PROJECT_NAME).data $(PROJECT_NAME).html $(PROJECT_NAME).js $(PROJECT_NAME).wasm index.html

run: all
	LD_LIBRARY_PATH=$(RAYLIB_PATH)/lib ./$(PROJECT_NAME)

webserve:
	python3 -m http.server 8000

zip: 
	rm -f tailgunner$(ZIP_NUMBER).zip
	zip -9 tailgunner$(ZIP_NUMBER).zip index.html tailgunner.data tailgunner.js tailgunner.wasm

# ============================================================================
# Analysis Targets - Memory Leaks and Static Issues
# ============================================================================

# 1. AddressSanitizer (ASAN) - Detects memory errors at runtime
asan: clean
	@echo "Building with AddressSanitizer..."
	$(MAKE) all ASAN=1

asan-run: asan
	@echo "Running with AddressSanitizer..."
	LD_LIBRARY_PATH=$(RAYLIB_PATH)/lib ./$(PROJECT_NAME)

# 2. Valgrind - Memory profiler for runtime leak detection
valgrind: all
	@echo "Running Valgrind memory profiler..."
	@command -v valgrind >/dev/null 2>&1 || (echo "Error: valgrind not installed. Run: sudo apt install valgrind" && exit 1)
	LD_LIBRARY_PATH=$(RAYLIB_PATH)/lib valgrind \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=valgrind-report.txt \
		./$(PROJECT_NAME)
	@echo "Valgrind report saved to valgrind-report.txt"

# 3. cppcheck - Static analysis for C/C++
cppcheck:
	@echo "Running cppcheck static analysis..."
	@command -v cppcheck >/dev/null 2>&1 || (echo "Error: cppcheck not installed. Run: sudo apt install cppcheck" && exit 1)
	# Provide include paths so cppcheck can resolve project and third-party headers
	cppcheck --enable=all --suppress=missingIncludeSystem --std=c99 \
		-I$(RAYLIB_NATIVE_PATH)/include \
		-I$(RAYLIB_EMSCRIPTEN_PATH)/src \
		-I$(EMSDK_PATH)/upstream/emscripten/system/include \
		--output-file=cppcheck-report.txt \
		$(SRC_DIR)
	@echo "cppcheck report saved to cppcheck-report.txt"
	@echo "Filtering to cppcheckfrom cppcheck report..."
	# remove known issues (3 lines each) to cppcheck-filtered-report.txt
	@sed '/Too many #ifdef configurations/{N;N;d};/src\/cJSON\.c/{N;N;d};/include\/raymath\.h/{N;N;d};/src\/external\/glad\.h/{N;N;d};/include\/rlgl\.h/{N;N;d}' cppcheck-report.txt > cppcheck-filtered-report.txt

# 4. clang-check - LLVM static analyzer (simpler version available)
clang-check:
	@echo "Running clang-check static analysis..."
	@command -v clang-check >/dev/null 2>&1 || (echo "Error: clang-check not installed. Run: sudo apt install clang-tools" && exit 1)
	@for file in $(wildcard $(SRC_DIR)/*.c); do \
		echo "Checking $$file..."; \
		clang-check $$file -- $(CFLAGS) $(INCLUDE_PATHS) -o /dev/null 2>&1 || true; \
	done

# 5. scan-build (Clang Static Analyzer) - Generates HTML reports
scan-build: clean
	@echo "Running scan-build (Clang Static Analyzer)..."
	@command -v scan-build >/dev/null 2>&1 || (echo "Error: scan-build not installed. Run: sudo apt install clang-tools" && exit 1)
	scan-build -o scan-build-report $(MAKE) all
	@echo "HTML report generated in: scan-build-report/"

# 6. GCC warnings - Enhanced compiler warnings (now default)
gcc-warnings: clean
	@echo "Building with enhanced GCC warnings..."
	$(MAKE) all EXTRA_WARNINGS=1

# Meta target to run all analysis tools
analyze: cppcheck clang-tidy valgrind
	@echo "Analysis complete!"

-include $(OBJS:.o=.d)
