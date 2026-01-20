# Tetris Project - IN204-TETRIS

A fully-featured Tetris implementation with SFML graphics, featuring single-player modes, AI opponents, and multiplayer support.

## Features

- **Single Player Mode**: Classic Tetris gameplay with level progression
- **Deathrun Mode**: Accelerating difficulty mode where pieces fall faster over time
- **AI Opponents**: Multiple difficulty levels (Simple and Advanced AI)
- **Multiplayer**: Online 2-player mode with simultaneous input handling
- **MVC Architecture**: Clean separation of concerns (Model-View-Controller)


## Screenshots

_(Add screenshots of your game here)_

## Prerequisites

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libfreetype-dev \
    libflac-dev \
    libvorbis-dev \
    libopenal-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev
```

### macOS

```bash
brew install cmake
# SFML will be downloaded automatically during build
```

### Windows

- Install [CMake](https://cmake.org/download/) (3.11 or higher)
- Install [Git](https://git-scm.com/download/win)
- Install a C++ compiler (Visual Studio 2019+ or MinGW)
- SFML will be downloaded automatically during build

## Building

### Clone the Repository

```bash
git clone <your-repository-url>
cd in204-tetris
```

### Build Instructions

```bash
# Configure the project
cmake -B build

# Build the project
cmake --build build

# For Release build (optimized)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Build Output

The executable will be located at:
- Linux/macOS: `build/IN204-TETRIS`
- Windows: `build/IN204-TETRIS.exe` or `build/Release/IN204-TETRIS.exe`

**Note**: The first build will take longer (5-10 minutes) as it downloads and compiles SFML from source. Subsequent builds will be much faster.

## Running

```bash
# Linux/macOS
./build/IN204-TETRIS

# Windows
build\IN204-TETRIS.exe
```

## Game Modes

### Single Player
- Classic Tetris gameplay
- Level-based progression (level increases every 10 lines cleared)
- Score tracking with formula: `score = base_points * (level + 1)`
  - 1 line: 40 points
  - 2 lines: 100 points
  - 3 lines: 300 points
  - 4 lines: 1200 points

### Deathrun Mode
- Accelerating fall speed
- Survival challenge
- Pieces fall faster continuously until game over

### AI Mode
- Play against AI opponents
- Multiple difficulty levels:
  - **Simple AI**: Heuristic-based placement
  - **Advanced AI**: Look-ahead with sophisticated evaluation

### Multiplayer
- Online 2-player mode
- Simultaneous input handling
- Real-time synchronization
- Each player has their own board

## Controls

### Player 1
- **Left Arrow**: Move piece left
- **Right Arrow**: Move piece right
- **Down Arrow**: Soft drop (accelerate fall)
- **Up Arrow / Space**: Rotate piece clockwise

### Player 2 (Multiplayer)
- _(To be defined)_

## Project Structure

```
in204-tetris/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── src/
│   ├── main.cpp            # Entry point
│   ├── model/              # Game logic (MVC)
│   │   ├── board.h/cpp
│   │   ├── tetromino.h/cpp
│   │   ├── gamestate.h/cpp
│   │   ├── score.h/cpp
│   │   └── level.h/cpp
│   ├── view/               # Rendering (MVC)
│   │   ├── gameview.h/cpp
│   │   └── menuview.h/cpp
│   ├── controller/         # Input handling (MVC)
│   │   ├── gamecontroller.h/cpp
│   │   └── inputhandler.h/cpp
│   ├── ai/                 # AI implementation
│   │   ├── aiplayer.h/cpp
│   │   ├── simpleai.h/cpp
│   │   └── advancedai.h/cpp
│   └── network/            # Network multiplayer
│       ├── networkmanager.h/cpp
│       ├── gameserver.h/cpp
│       └── gameclient.h/cpp
├── data/                   # Game assets
├── headers/                # Additional headers
├── lib/                    # External libraries
└── test/                   # Unit tests
```

## Architecture

This project follows the **MVC (Model-View-Controller)** architecture:

- **Model**: Game state, board logic, tetromino management, scoring
  - `Board`: Grid management and collision detection
  - `Tetromino`: Piece representation and rotation
  - `GameState`: Overall game state management
  - `Score`: Scoring system
  - `Level`: Level progression

- **View**: SFML rendering, UI display
  - `GameView`: Main game rendering
  - `MenuView`: Menu and UI rendering

- **Controller**: Input handling, game flow control
  - `GameController`: Main game loop and logic
  - `InputHandler`: Keyboard input processing

## Dependencies

- **SFML 3.0.2**: Graphics, window management, networking
  - Automatically downloaded and built via CMake FetchContent
  - No manual installation required
- **CMake 3.11+**: Build system
- **C++17**: Compiler standard

## Development Roadmap

### Day 1: Foundation & Core MVC Setup ✓
- [x] SFML integration
- [x] Basic MVC structure
- [x] Window management and rendering loop

### Day 2: Complete Single-Player Tetris
- [ ] All 7 tetromino types
- [ ] Complete rotation system
- [ ] Line clearing and scoring
- [ ] Game over detection

### Day 3: Level System & Deathrun Mode
- [ ] Level progression system
- [ ] Deathrun mode implementation
- [ ] Menu system

### Day 4: AI Implementation
- [ ] Simple AI (easy difficulty)
- [ ] Advanced AI (hard difficulty)
- [ ] AI integration into game modes

### Day 5: Network Multiplayer Infrastructure
- [ ] Network protocol design
- [ ] Server implementation
- [ ] Client implementation
- [ ] Multiplayer game state

### Day 6: Multiplayer Polish & Integration
- [ ] Simultaneous input handling
- [ ] Multiplayer UI
- [ ] Game synchronization
- [ ] Final testing and polish

## Troubleshooting

### CMake Configuration Fails

**Error: "Could NOT find X11"**
```bash
sudo apt install libx11-dev libxrandr-dev libxcursor-dev libxi-dev
```

**Error: "SFML not found"**
- SFML is downloaded automatically. Check your internet connection.
- If issues persist, try cleaning the build directory:
  ```bash
  rm -rf build
  cmake -B build
  ```

**Error: "CMake Error: Could not find SFML"**
- Ensure you have all system dependencies installed (see Prerequisites)
- Check that CMake version is 3.11 or higher: `cmake --version`

### Build Errors

**"Undefined reference to sf::..."**
- Ensure all SFML modules are linked in CMakeLists.txt
- Clean and rebuild:
  ```bash
  rm -rf build
  cmake -B build
  cmake --build build
  ```

**"No source files found"**
- Ensure you have `.cpp` files in the `src/` directory
- Check that `main.cpp` exists in `src/`

### Runtime Issues

**Window doesn't open**
- Check if X11 is running (Linux): `echo $DISPLAY`
- Verify graphics drivers are installed
- On WSL2, ensure X11 forwarding is set up

**Game runs but crashes**
- Check that all required source files are present
- Verify SFML was built successfully (check `build/_deps/sfml-src/`)

## Contributing

This is a university project (IN204). Contributions and suggestions are welcome!

## License

_(Add your license here)_

## Authors

- Your Name

## Acknowledgments

- SFML team for the excellent graphics library
- Tetris Guideline for game rules reference
- B. Monsuez for project specifications

## References

- [SFML Documentation](https://www.sfml-dev.org/documentation/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Tetris Guideline](https://tetris.fandom.com/wiki/Tetris_Guideline)
- Project PDF: `tetris_-_2023.pdf`
