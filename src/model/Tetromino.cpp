#include "Tetromino.h"
#include <algorithm>

// Wall kick data for SRS (Super Rotation System)
// Format: {x_offset, y_offset} for each test case
// Order matters - test in sequence until one works

// Wall kicks for J, L, S, T, Z pieces (non-I pieces)
// Transition: 0->1 (spawn->right), 1->0, 1->2, 2->1, 2->3, 3->2, 3->0, 0->3
namespace WallKicks {
    // Standard wall kicks for J, L, S, T, Z (0->1 and 1->0)
    static const std::vector<Point> JLSTZ_01 = {{0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2}};
    static const std::vector<Point> JLSTZ_10 = {{0,0}, {1,0}, {1,-1}, {0,2}, {1,2}};
    
    // Standard wall kicks for J, L, S, T, Z (1->2 and 2->1)
    static const std::vector<Point> JLSTZ_12 = {{0,0}, {1,0}, {1,-1}, {0,2}, {1,2}};
    static const std::vector<Point> JLSTZ_21 = {{0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2}};
    
    // Standard wall kicks for J, L, S, T, Z (2->3 and 3->2)
    static const std::vector<Point> JLSTZ_23 = {{0,0}, {1,0}, {1,1}, {0,-2}, {1,-2}};
    static const std::vector<Point> JLSTZ_32 = {{0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2}};
    
    // Standard wall kicks for J, L, S, T, Z (3->0 and 0->3)
    static const std::vector<Point> JLSTZ_30 = {{0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2}};
    static const std::vector<Point> JLSTZ_03 = {{0,0}, {1,0}, {1,-1}, {0,2}, {1,2}};
    
    // I-piece wall kicks (different from standard)
    // 0->1 and 1->0
    static const std::vector<Point> I_01 = {{0,0}, {-2,0}, {1,0}, {-2,-1}, {1,2}};
    static const std::vector<Point> I_10 = {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2}};
    
    // 1->2 and 2->1
    static const std::vector<Point> I_12 = {{0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1}};
    static const std::vector<Point> I_21 = {{0,0}, {1,0}, {-2,0}, {1,-2}, {-2,1}};
    
    // 2->3 and 3->2
    static const std::vector<Point> I_23 = {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2}};
    static const std::vector<Point> I_32 = {{0,0}, {-2,0}, {1,0}, {-2,-1}, {1,2}};
    
    // 3->0 and 0->3
    static const std::vector<Point> I_30 = {{0,0}, {1,0}, {-2,0}, {1,-2}, {-2,1}};
    static const std::vector<Point> I_03 = {{0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1}};
    
    // O-piece has no wall kicks (it's a square, rotation doesn't change shape)
    static const std::vector<Point> O_NONE = {{0,0}};
}

Tetromino::Tetromino(TetrominoType type)
    : m_type(type),
      m_rotationState(RotationState::R0) {
    
    // Initialize color
    switch (type) {
        case TetrominoType::I: m_colorId = 1; break;
        case TetrominoType::J: m_colorId = 2; break;
        case TetrominoType::L: m_colorId = 3; break;
        case TetrominoType::O: m_colorId = 4; break;
        case TetrominoType::S: m_colorId = 5; break;
        case TetrominoType::T: m_colorId = 6; break;
        case TetrominoType::Z: m_colorId = 7; break;
    }
    
    initializeRotations();
}

void Tetromino::initializeRotations() {
    // Define spawn orientation (rotation 0) for each piece
    std::vector<Point> spawn;
    
    switch (m_type) {
        case TetrominoType::I:
            spawn = {{-1,0}, {0,0}, {1,0}, {2,0}};
            break;
        case TetrominoType::J:
            spawn = {{-1,-1}, {-1,0}, {0,0}, {1,0}};
            break;
        case TetrominoType::L:
            spawn = {{1,-1}, {-1,0}, {0,0}, {1,0}};
            break;
        case TetrominoType::O:
            spawn = {{0,0}, {1,0}, {0,1}, {1,1}};
            break;
        case TetrominoType::S:
            spawn = {{0,0}, {1,0}, {-1,1}, {0,1}};
            break;
        case TetrominoType::T:
            spawn = {{-1,0}, {0,0}, {1,0}, {0,1}};
            break;
        case TetrominoType::Z:
            spawn = {{-1,0}, {0,0}, {0,1}, {1,1}};
            break;
    }
    
    // Store spawn state
    m_rotationStates[0] = spawn;
    
    // Generate other rotation states by rotating spawn
    if (m_type == TetrominoType::O) {
        // O-piece is the same in all rotations
        m_rotationStates[1] = spawn;
        m_rotationStates[2] = spawn;
        m_rotationStates[3] = spawn;
    } else {
        // Rotate spawn to get other states
        m_rotationStates[1] = spawn;
        for (auto& p : m_rotationStates[1]) {
            p = rotatePointClockwise(p);
        }
        
        m_rotationStates[2] = m_rotationStates[1];
        for (auto& p : m_rotationStates[2]) {
            p = rotatePointClockwise(p);
        }
        
        m_rotationStates[3] = m_rotationStates[2];
        for (auto& p : m_rotationStates[3]) {
            p = rotatePointClockwise(p);
        }
    }
}

Point Tetromino::rotatePointClockwise(const Point& p) {
    // Rotate 90° clockwise: (x, y) -> (-y, x)
    return Point(-p.y, p.x);
}

Point Tetromino::rotatePointCounterClockwise(const Point& p) {
    // Rotate 90° counter-clockwise: (x, y) -> (y, -x)
    return Point(p.y, -p.x);
}

TetrominoType Tetromino::getType() const {
    return m_type;
}

int Tetromino::getColorId() const {
    return m_colorId;
}

RotationState Tetromino::getRotationState() const {
    return m_rotationState;
}

const std::vector<Point>& Tetromino::getBlocks() const {
    return m_rotationStates[static_cast<int>(m_rotationState)];
}

const std::vector<Point>& Tetromino::getBlocks(RotationState state) const {
    return m_rotationStates[static_cast<int>(state)];
}

void Tetromino::rotateClockwise() {
    int current = static_cast<int>(m_rotationState);
    m_rotationState = static_cast<RotationState>((current + 1) % 4);
}

void Tetromino::rotateCounterClockwise() {
    int current = static_cast<int>(m_rotationState);
    m_rotationState = static_cast<RotationState>((current + 3) % 4); // +3 is same as -1 mod 4
}

void Tetromino::setRotationState(RotationState state) {
    m_rotationState = state;
}

const std::vector<Point>& Tetromino::getWallKicks(TetrominoType type, RotationState from, RotationState to) {
    int fromInt = static_cast<int>(from);
    int toInt = static_cast<int>(to);
    
    if (type == TetrominoType::I) {
        // I-piece wall kicks
        if (fromInt == 0 && toInt == 1) return WallKicks::I_01;
        if (fromInt == 1 && toInt == 0) return WallKicks::I_10;
        if (fromInt == 1 && toInt == 2) return WallKicks::I_12;
        if (fromInt == 2 && toInt == 1) return WallKicks::I_21;
        if (fromInt == 2 && toInt == 3) return WallKicks::I_23;
        if (fromInt == 3 && toInt == 2) return WallKicks::I_32;
        if (fromInt == 3 && toInt == 0) return WallKicks::I_30;
        if (fromInt == 0 && toInt == 3) return WallKicks::I_03;
    } else if (type == TetrominoType::O) {
        // O-piece doesn't need wall kicks
        return WallKicks::O_NONE;
    } else {
        // J, L, S, T, Z wall kicks
        if (fromInt == 0 && toInt == 1) return WallKicks::JLSTZ_01;
        if (fromInt == 1 && toInt == 0) return WallKicks::JLSTZ_10;
        if (fromInt == 1 && toInt == 2) return WallKicks::JLSTZ_12;
        if (fromInt == 2 && toInt == 1) return WallKicks::JLSTZ_21;
        if (fromInt == 2 && toInt == 3) return WallKicks::JLSTZ_23;
        if (fromInt == 3 && toInt == 2) return WallKicks::JLSTZ_32;
        if (fromInt == 3 && toInt == 0) return WallKicks::JLSTZ_30;
        if (fromInt == 0 && toInt == 3) return WallKicks::JLSTZ_03;
    }
    
    // Default: no offset
    return WallKicks::O_NONE;
}
