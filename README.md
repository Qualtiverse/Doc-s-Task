# Doc's Task - Time Manipulation Puzzle Game

A physics-based puzzle game where you record object movements, then replay them to solve puzzles cooperatively with your past self.

## Core Mechanic

**Record → Replay → Solve**

1. **Record (R)**: Physics objects fall, swing, or move - their transforms are captured per frame
2. **Replay (P)**: Objects repeat your recorded actions exactly
3. **Interact**: Use replayed objects as platforms, bridges, or triggers
4. **Loop**: Record on top of replays to create complex chains

## Controls

| Key | Action |
|-----|--------|
| WASD | Move |
| SPACE | Jump |
| **R** | Start/Stop Recording |
| **P** | Start Replay |
| **T** | Stop Time / Toggle Replay |
| Mouse Drag (Timeline) | Scrub through recording |
| TAB | Next Level |
| ESC | Pause/Menu |
| CTRL+R | Restart Level |
| H | Toggle Help |

## Levels

1. **Recording Basics** - Drop a box on a button, replay to hold it
2. **Timing** - Record a swinging platform at the right moment
3. **Multi-Object** - Record multiple boxes to build stairs
4. **Loop Puzzle** - Record, replay, then record again on top
5. **Final Test** - Combine all mechanics

## Build Requirements

- CMake 3.16+
- C++17 compiler (MSVC, GCC, Clang)
- Raylib 5.5+ (fetched automatically via CMake)

## Building

```bash
# Configure
cmake -B build -S .

# Build
cmake --build build --config Release

# Run
./build/DocTask           # Linux/macOS
./build/Release/DocTask.exe  # Windows
```

## Project Structure

```
src/
├── core/           # Game systems (physics, recording, levels)
├── ui/             # Timeline, menus, HUD
└── levels/         # Puzzle level implementations
```

## License

MIT License - Feel free to use for learning or projects!