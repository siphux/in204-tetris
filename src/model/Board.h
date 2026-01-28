#pragma once
#include <vector>

struct Point;

//Tetris board class managing grid state
class Board {
public:
    static constexpr int Width  = 10;
    static constexpr int Height = 21; //one more row for the hidden spawn area

    Board();

    // reset the board
    void clear();
    // Check if coordinates are inside the board (here x and y are horizontal and vertical indices != m_grid[y][x])
    bool isInside(int x, int y) const;
    // Check if a cell is empty
    bool isEmpty(int x, int y) const;
    // Read-only access to a cell
    int getCell(int x, int y) const;
    // Modify a cell value
    void setCell(int x, int y, int value);
    


    //check if blocks yield collision at given position
    bool checkCollision(const std::vector<Point>& blocks, int posX, int posY) const;

private:
    //initial grid a matrix 
    int m_grid[Height][Width];
};
