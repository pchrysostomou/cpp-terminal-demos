# C++ Terminal Demos

> Three fully interactive C++ projects that run entirely inside your terminal — with true ANSI colors, Unicode graphics, real-time physics, and keyboard input.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square&logo=cplusplus)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux-lightgrey?style=flat-square)
![Build](https://img.shields.io/badge/build-make-green?style=flat-square)

---

## Projects

### 1. Ray Tracer — `raytracer/`

An animated ASCII ray tracer that renders a full 3D scene directly in the terminal.

```
  ██████╗  █████╗ ██╗   ██╗    ████████╗██████╗  █████╗  ██████╗███████╗██████╗
  ██╔══██╗██╔══██╗╚██╗ ██╔╝       ██╔══╝██╔══██╗██╔══██╗██╔════╝██╔════╝██╔══██╗
  ██████╔╝███████║ ╚████╔╝        ██║   ██████╔╝███████║██║     █████╗  ██████╔╝
```

**Features:**
- Phong shading (ambient + diffuse + specular)
- Recursive reflections (up to 4 levels deep)
- Shadow rays with soft occlusion
- 2×2 supersampling anti-aliasing
- Animated scene — spheres orbit, dance, and glow
- Checkerboard floor with reflections
- Sky gradient background
- 24-bit ANSI true-color output
- Auto-detects terminal size

**Build & Run:**
```bash
cd raytracer
g++ -O2 -std=c++17 -o raytracer raytracer.cpp
./raytracer
# Optional: custom size
./raytracer 110 40
```

---

### 2. Fireworks Simulator — `fireworks/`

A real-time particle physics simulation with 5 explosion patterns, gravity, drag, and a grand finale.

**Features:**
- 5 unique explosion patterns:
  - **Burst** — classic scatter with fast outliers
  - **Willow** — drooping arcs that curve like a willow tree
  - **Ring** — tight circular shell
  - **Chrysanthemum** — double concentric rings
  - **Glitter** — slow-falling diamond sparks
- Realistic physics: gravity, air drag, velocity decay
- Particles shift from bright white → ember orange as they die
- 45 twinkling background stars
- **Grand Finale** at frame 600 — 3–6 simultaneous fireworks
- Multi-file architecture (8 source files)

**Build & Run:**
```bash
cd fireworks
make
./fireworks
# Ctrl+C to exit
```

**File Structure:**
```
fireworks/
├── Makefile
├── math.hpp        ← Vec2 + random utilities
├── color.hpp       ← Color struct + HSV conversion
├── particle.hpp/cpp
├── firework.hpp/cpp  ← 5 explosion patterns
├── scene.hpp/cpp     ← spawn logic + grand finale
├── renderer.hpp/cpp  ← ANSI cell buffer renderer
└── main.cpp
```

---

### 3. Space Invaders — `space_invaders/`

A fully playable Space Invaders game in the terminal with keyboard input, score tracking, and multiple waves.

```
  /^\  oOo  (o)  [.]      ← 4 enemy types
   \^/                     ← your ship
```

**Features:**
- Real keyboard input via `termios` (no Enter needed)
- 44 enemies (11×4 grid) that march and speed up as they die
- Enemies shoot back — from the lowest alive enemy per column
- 3 lives with invincibility frames after being hit
- Score system: top rows worth more points
- Hi-Score saved across rounds
- Wave system — each wave is faster and shoots more
- Animated explosions on enemy/player death
- Twinkling starfield background
- Title screen + Game Over screen

**Build & Run:**
```bash
cd space_invaders
make
./invaders
```

**Controls:**
| Key | Action |
|-----|--------|
| `←` `→` or `A` `D` | Move left / right |
| `SPACE` or `W` | Shoot |
| `R` | Restart (after game over) |
| `Q` / `ESC` | Quit |

**File Structure:**
```
space_invaders/
├── Makefile
├── color.hpp         ← ANSI 24-bit color
├── input.hpp/cpp     ← non-blocking termios keyboard
├── game.hpp/cpp      ← all game logic + physics
├── renderer.hpp/cpp  ← cell buffer + HUD rendering
└── main.cpp
```

---

## Requirements

| Requirement | Details |
|-------------|---------|
| Compiler | `g++` with C++17 support |
| Terminal | Must support 24-bit ANSI color (iTerm2, macOS Terminal, most Linux terminals) |
| Font | UTF-8 encoding required (for block characters: █ ▓ ▒ ░) |
| OS | macOS or Linux |

---

## Quick Start

```bash
git clone https://github.com/pchrysostomou/cpp-terminal-demos.git
cd cpp-terminal-demos

# Ray Tracer
cd raytracer && g++ -O2 -std=c++17 -o raytracer raytracer.cpp && ./raytracer

# Fireworks
cd ../fireworks && make && ./fireworks

# Space Invaders
cd ../space_invaders && make && ./invaders
```

---

## Technical Highlights

- **Zero dependencies** — only the C++17 standard library + POSIX (`termios`, `ioctl`)
- **Double-buffered rendering** — builds a full frame string in memory, flushes once per frame to avoid flicker
- **Optimized ANSI output** — only emits a new color escape when the color changes from the previous cell
- **Non-blocking input** — uses `VMIN=0, VTIME=0` raw mode so keyboard events are polled every frame without blocking
- **Reserve-based vector safety** — prevents iterator/reference invalidation during particle push_back

---

*Built with C++17 — no game engine, no graphics library, just the terminal.*
