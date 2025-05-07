# Apple Catcher: Sheep Grow Edition 🍎🐑

A classic 2D apple-catching game built with C++ and OpenGL/freeGLUT, featuring a fun twist: mischievous sheep that wander the pasture, eat fallen apples, and grow bigger!

![Screenshot 2025-05-07 164036](https://github.com/user-attachments/assets/db83ef85-d3d8-48a7-857a-634e6fb64908)
![Screenshot 2025-05-07 164020](https://github.com/user-attachments/assets/18cd9567-8fc3-4efd-88ef-0e925d5a33e7)


## Game Overview

The player controls a basket to catch apples falling from the sky. The goal is to catch a certain number of apples to win, while avoiding missing too many. Adding to the challenge, sheep roam the ground. If an apple falls and isn't caught, a nearby sheep might decide it's a tasty snack, eat it, and grow larger!

## Features

*   **Classic Apple Catching:** Move your basket to catch falling apples.
*   **Scoring System:** Gain points for each apple caught.
*   **Win/Lose Conditions:**
    *   Win by reaching the target score (`WIN_SCORE`).
    *   Lose if too many apples are missed (`LOSE_LIMIT`).
*   **Interactive Sheep:**
    *   Sheep wander the game area.
    *   They detect and move towards fallen apples within their range.
    *   Sheep "eat" fallen apples, removing them from play.
    *   **Sheep grow in size** each time they successfully eat an apple!
*   **Dynamic Apple Spawning:** Apples spawn at intervals with some randomization in position and speed.
*   **Visual Elements:**
    *   Cute sheep with a simple bobbing animation.
    *   Decorations like a sun, moving clouds, flowers, and mushrooms.
    *   Gradient sky background.
*   **Game Controls:** Pause, restart, and exit functionality.
*   **Basic Physics:** Apples fall, sheep move towards targets.

## Gameplay Controls

*   **Arrow Keys (Left/Right):** Move the basket horizontally.
*   **Arrow Keys (Up/Down):** Move the basket vertically (within limits).
*   **`P` Key:** Pause / Resume the game.
*   **`R` Key:** Restart the game (if game over or paused).
*   **`Esc` Key:** Exit the game.

## Technology Used

*   **Language:** C++
*   **Graphics API:** OpenGL
*   **Windowing/Input:** freeGLUT

## Getting Started

### Prerequisites

*   A C++ compiler (like G++).
*   OpenGL libraries.
*   freeGLUT development libraries.
    *   On Debian/Ubuntu: `sudo apt-get install freeglut3-dev`
    *   On Fedora: `sudo dnf install freeglut-devel`
    *   On macOS (with Homebrew): `brew install freeglut`
    *   For Windows: You might use MinGW/MSYS2 and install the `freeglut` package, or link against pre-compiled libraries.

### Compilation

Navigate to the directory containing `your_game_file.cpp` (replace with the actual .cpp file name) and compile using a command similar to this:

```bash
g++ your_game_file.cpp -o AppleCatcherSheepGrow -lGL -lGLU -lglut -lm
