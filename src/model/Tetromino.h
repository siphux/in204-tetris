#pragma once
#include <vector>
#include <array>

// Simple 2D integer point
struct Point {
    int x;
    int y;
    
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
};

// All possible tetromino types
enum class TetrominoType {
    I, J, L, O, S, T, Z
};

// Rotation state: 0=0°, 1=90°, 2=180°, 3=270°
enum class RotationState {
    R0 = 0,
    R90 = 1,
    R180 = 2,
    R270 = 3
};

// Represents the shape of a tetromino with SRS (Super Rotation System).
// Does NOT know about the board or collisions.
class Tetromino {
public:
    explicit Tetromino(TetrominoType type = TetrominoType::O);

    TetrominoType getType() const;
    int getColorId() const;
    RotationState getRotationState() const;

    // Get blocks for current rotation state
    const std::vector<Point>& getBlocks() const;
    
    // Get blocks for a specific rotation state
    const std::vector<Point>& getBlocks(RotationState state) const;
    
    // Rotate clockwise (0->1->2->3->0)
    void rotateClockwise();
    
    // Rotate counter-clockwise (0->3->2->1->0)
    void rotateCounterClockwise();
    
    // Set rotation state directly
    void setRotationState(RotationState state);
    
    // Get wall kick offsets for rotation transition
    // Returns array of (x, y) offsets to try in order
    static const std::vector<Point>& getWallKicks(TetrominoType type, RotationState from, RotationState to);

private:
    TetrominoType m_type;
    int m_colorId;
    RotationState m_rotationState;
    
    // Store all 4 rotation states for this piece
    std::array<std::vector<Point>, 4> m_rotationStates;
    
    // Initialize all rotation states for the piece type
    void initializeRotations();
    
    // Helper to rotate a point 90° clockwise around origin
    static Point rotatePointClockwise(const Point& p);
    
    // Helper to rotate a point 90° counter-clockwise around origin
    static Point rotatePointCounterClockwise(const Point& p);
};
