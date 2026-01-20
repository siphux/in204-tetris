#pragma once

// Represents the Tetris grid.
// The board only stores fixed blocks (no moving piece).
class Board {
public:
    static constexpr int Width  = 10;
    static constexpr int Height = 21; // includes hidden spawn row

    Board();

    // Reset the board to empty
    void clear();

    // Check if coordinates are inside the board
    bool isInside(int x, int y) const;

    // Check if a cell is empty
    bool isEmpty(int x, int y) const;

    // Read-only access to a cell
    int getCell(int x, int y) const;

    // Modify a cell value
    void setCell(int x, int y, int value);

private:
    // Grid storing color IDs (0 = empty)
    int m_grid[Height][Width];
};
