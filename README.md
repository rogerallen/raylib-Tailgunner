# Raylib Tailgunner Clone

Mostly vibe-coded via gemini-2.5-pro over 2 days.  I'm a bit astonished, honestly.  Now starting to mix 
my own coding with gemini's.

Controls: **left click** to fire, **space** to activate the force field, Enter or click to start/continue.

<p align="center">
	<img src="resources/screenshot.png" alt="Tailgunner screenshot" width="800" />
</p>

## Building & running

### Linux Native Build

Look in makefile and point RAYLIB_NATIVE_PATH to the right spot.  You will need a native build of raylib.  Download a build of raylib from [here](https://github.com/raysan5/raylib/releases).

Make sure libcurl, libssl headers are installed: `sudo apt install libcurl4-openssl-dev`

```
make
make run
```

### Emscripten Build and Serve

Look in the Makefile and point EMSDK_PATH and RAYLIB_EMSCRIPTEN_PATH to the right spot.  You will need a build of emscripten raylib.

```
make web
make webserve
```
Now open http://localhost:8000/tailgunner.html and you should see the game.

## Development Notes

Recommended Analysis Workflow

### 1. Daily Development
```bash
make gcc-warnings    # Fast, catches common issues
make asan-run        # Build and test with memory instrumentation
```

### 2. Before Commit
```bash
make cppcheck        # Static analysis
make clang-check     # Clang validation
```

### 3. Before Release
```bash
make valgrind        # Thorough memory check
make analyze         # Run all tools (cppcheck, clang-check, valgrind)
```

### 4. Debugging Memory Issues
```bash
make asan-run        # First: Fast runtime detection
# If ASAN finds nothing:
make valgrind        # Second: More thorough analysis
```
