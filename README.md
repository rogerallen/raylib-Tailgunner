# Raylib Tailgunner Clone

Mostly vibe-coded via gemini-2.5-pro over 2 days.  I'm a bit astonished, honestly.

Fix the RAYLIB_PATH in the Makefile if you want to try this out.  Download a build of raylib from [here](https://github.com/raysan5/raylib/releases).

<p align="center">
	<img src="resources/screenshot.png" alt="Tailgunner screenshot" width="800" />
</p>

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
- Controls: left click to fire, space to activate the force field, Enter or click to start/continue.

If you plan to extend or refactor the project, consider making the managers opaque (create/destroy
APIs) and moving audio handling fully to `main.c` so subsystems don't call `PlaySound` directly.
