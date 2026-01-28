# IN204 Tetris Project

A Tetris implementation with single-player modes, AI opponents, and network multiplayer support.

## Features

### Game Modes
- **Level Mode**: Classic Tetris with increasing difficulty levels
- **AI Modes**: Either watch an AI play (Simple or Advanced), play against AI opponents or watch two AIs play against each other 
- **Multiplayer Mode**: 1v1 gameplay with marathon mode where first to clear a target number of lines wins

### LAN Multiplayer Features (not working in WSL to be tested elsewhere)
- Local network multiplayer 
- Split-screen display showing both players' boards
- Real-time connection status and latency display
- Client-side prediction for smooth gameplay
- Server-side input validation
- Input buffering for network latency compensation

## Building

### Prerequisites
- CMake 3.10 or higher
- C++ compiler which uses at least C++17
- SFML 3.0.2

### Build Instructions (for Linux/MacOS)

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd in204-tetris
   ```

2. **Configure the build**:
   ```bash
   cmake -B build -S .
   ```

3. **Build the project**:
   ```bash
   cmake --build build
   ```


4. **Run the game**:
   ```bash
   ./build/IN204-TETRIS
   ```



## Controls

- **Left Arrow**: Move piece left
- **Right Arrow**: Move piece right
- **Down Arrow**: Soft drop
- **Up Arrow**: Rotate clockwise
- **Z**: Rotate counter-clockwise
- **Space**: Hard drop
- **Escape**: Pause menu

## Multiplayer Setup

### Local Network (Same Sub-Network)

1. **Host**: Select "Multiplayer" → "Host Game"
2. **Note the Local Network IP** displayed (e.g., `192.168.1.100:53000`)
3. **Client**: Select "Multiplayer" → "Join Game" → "Local Network"
4. **Enter the host's Local Network IP** and connect

### Internet Play (Different Networks)

1. **Host**: 
   - Select "Multiplayer" → "Host Game"
   - Note the **Internet IP** displayed (e.g., `123.45.67.89:53000`)
   - **Port Forward**: Forward TCP port 53000 on your router to this computer
   - Share the Internet IP with your friend

2. **Client**:
   - Select "Multiplayer" → "Join Game" → "Internet"
   - Enter the host's **Internet IP** (with optional port, e.g., `123.45.67.89:53000`)
   - Connect

### Port Forwarding Instructions

1. Access your router's admin page (usually `192.168.1.1` or `192.168.0.1`)
2. Find "Port Forwarding" or "Virtual Server" settings
3. Forward TCP port **53000** to your computer's local IP address
4. Common router addresses: `192.168.1.1` or `192.168.0.1`

## Project Structure

```
in204-tetris/
├── src/
│   ├── controller/     # Game controller and input handling
│   ├── model/          # Game logic, board, pieces, modes
│   ├── view/           # Rendering and UI
│   ├── network/        # Multiplayer networking
│   ├── ai/             # AI opponents
│   └── main.cpp        # Entry point
├── CMakeLists.txt      # CMake build configuration
├── data/               # Contains file for game music and possibly other assets
├── config.ini          # Configuration file
└── README.md           # This file
```

## Technical Details

### Network Protocol
- Uses TCP sockets via SFML Network
- Custom message protocol with frame numbers for synchronization

### Architecture
- MVC (Model-View-Controller) pattern
- Event-driven input handling
- State-based game modes
- Network session management

