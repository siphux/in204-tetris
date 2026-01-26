# IN204 Tetris Project

A feature-rich Tetris implementation with single-player modes, AI opponents, and network multiplayer support.

## Features

### Game Modes
- **Level Mode**: Classic Tetris with increasing difficulty levels
- **Deathrun Mode**: Time-based challenge mode
- **AI Mode**: Play against AI opponents (Simple or Advanced)
- **Multiplayer Mode**: Network-based 1v1 gameplay
  - **Race Mode**: First to clear target lines wins
  - **Malus Mode**: Attack opponents with maluses

### Multiplayer Features
- Local network and internet play support
- Split-screen display showing both players' boards
- Real-time connection status and latency display
- Client-side prediction for smooth gameplay
- Server-side input validation
- Input buffering for network latency compensation

## Building

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- SFML 3.0.2 or compatible

### Build Instructions

1. **Clone the repository** (if not already done):
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

   Or on Windows:
   ```bash
   cmake --build build --config Release
   ```

4. **Run the game**:
   ```bash
   ./build/IN204-TETRIS
   ```

   Or on Windows:
   ```bash
   build\Release\IN204-TETRIS.exe
   ```

### CMake Options

- `CMAKE_BUILD_TYPE`: Set to `Release` for optimized builds, `Debug` for debugging
  ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   ```

## Controls

- **Left Arrow**: Move piece left
- **Right Arrow**: Move piece right
- **Down Arrow**: Soft drop
- **Up Arrow**: Rotate clockwise
- **Z**: Rotate counter-clockwise
- **Space**: Hard drop
- **Escape**: Pause menu / Exit

## Multiplayer Setup

### Local Network (Same Wi-Fi)

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
├── CMakeLists.txt      # Build configuration
└── README.md          # This file
```

## Technical Details

### Network Protocol
- Uses TCP sockets via SFML Network
- Custom message protocol with frame numbers for synchronization
- Heartbeat system for connection monitoring
- Client-side prediction for responsive gameplay

### Architecture
- MVC (Model-View-Controller) pattern
- Event-driven input handling
- State-based game modes
- Network session management

## Troubleshooting

### Connection Issues
- **"Connection timeout"**: 
  - Check firewall settings
  - Verify port forwarding (for internet play)
  - Ensure both players are on the same network (for local play)
  - Try using the local IP instead of public IP

### Build Issues
- **SFML not found**: Ensure SFML is installed and CMake can find it
- **C++17 errors**: Update your compiler to a C++17 compatible version

## License

[Add your license information here]

## Contributors

[Add contributor information here]
