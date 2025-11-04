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

## Module-level notes

These notes describe the high-level organization of the code and the role of `src/main.c`.

- `main.c` is the application bootstrap and primary game loop. It is intentionally small: it
	initializes subsystems, runs input -> update -> render each frame, and performs teardown.
- Subsystems are encapsulated in small "manager" structs (stack-allocated in `main.c`):
	`EnemyManager`, `LaserManager`, and `ForceFieldManager`. Each subsystem exports `Init` /
	`Update` / `Draw` functions that accept a pointer to the manager instance. This avoids
	module-level globals and makes the state easier to reason about or move to heap if needed.
- Audio resources (sounds) are loaded in `main.c` and currently played by both `main.c` and
	some subsystems. Prefer playing audio at the caller level (top-level) for clearer separation
	of concerns — see `src/laser.c` for an example where a hit returns a count and the caller
	plays the explosion sound.
- The game loop order is: input handling (fire / forcefield), Update subsystems (lasers, starfield,
	enemies, forcefield), then Render (3D mode: starfield, enemies, lasers; 2D overlays and UI).
- Important configuration values (counts, lifetimes, radii) live in `src/config.h` so tuning is
	centralized.

