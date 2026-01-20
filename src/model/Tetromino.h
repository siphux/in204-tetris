#pragma once
#include <vector>

// Simple 2D integer point
struct Point {
    int x;
    int y;
};

// All possible tetromino types
enum class TetrominoType {
    I, J, L, O, S, T, Z
};

// Represents the shape of a tetromino.
// Does NOT know about the board or collisions.
class Tetromino {
public:
    explicit Tetromino(TetrominoType type = TetrominoType::O);

    TetrominoType getType() const;
    int getColorId() const;

    // Relative block positions
    const std::vector<Point>& getBlocks() const;

private:
    TetrominoType m_type;
    int m_colorId;
    std::vector<Point> m_blocks;
};
