#include "Board.h"
#include "Tetromino.h"  // For Point definition
#include <vector>

Board::Board() {
    clear();
}
//When x, y is passed, x corresponds to the row and y the column therefore, m_grid[y][x]!!!!
void Board::clear() {
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            m_grid[y][x] = 0;
        }
    }
}

bool Board::isInside(int x, int y) const {
    return x >= 0 && x < Width && y >= 0 && y < Height;
}

bool Board::isEmpty(int x, int y) const {
    return isInside(x, y) && m_grid[y][x] == 0;
}

int Board::getCell(int x, int y) const {
    return m_grid[y][x];
}


void Board::setCell(int x, int y, int value) {
    if (isInside(x, y)) {
        m_grid[y][x] = value;
    }
}

bool Board::checkCollision(const std::vector<Point>& blocks, int posX, int posY) const {
    for (const auto& block : blocks) {
        int x = posX + block.x;
        int y = posY + block.y;
        if (x < 0 || x >= Width) {
            return true;
        }
        if (y >= Height) {
            return true;
        }
        
        if (y >= 0 && m_grid[y][x] != 0) {
            return true;
        }
    }
    
    return false;
}
