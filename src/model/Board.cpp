#include "Board.h"

Board::Board() {
    clear();
}

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
