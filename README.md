# SkillshotLab

SkillshotLab is a small C++ playground for experimenting with skillshot prediction, geometry, and decision logic without depending on any game API.

This version includes a simple graphical view using SFML. It shows:

* The caster as a circle at the center left
* Moving targets as circles
* Predicted impact points and lines from the caster toward them

## Requirements

* A C++17 capable compiler
* CMake
* SFML 2.5 or newer

On macOS with Homebrew:

```bash
brew install sfml
```

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
./SkillshotLab
```

You should see a window with moving targets and predicted hit points updating in real time.
