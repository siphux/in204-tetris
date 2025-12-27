#include "board.h"

// mask to keep only the usefull WIDTH bits (10 bits here)
static constexpr uint16_t FULL_MASK = (1u << Board::WIDTH) - 1;

// constructor
Board::Board() {
    for (int i=0; i<Board::HEIGHT; i++){
        rows[i] = 0;
    }
}

// reading access to a row
uint16_t Board::getRow(int y) const {
    return rows[y];
}


/* Test if a piece can be placed
    mask   : mask 4x4 of the piece
    x, y   : position on the board (bottom left corner)
*/

bool Board::canPlace(uint16_t mask, int x, int y) const {
    for (int i=0; i<4; i++) {
        uint16_t pieceRow = (mask >> (i * 4)) & 0xF;

        if (pieceRow == 0)
            continue; // empty row

        int boardY = y + i;

        // over vertical boundaries
        if (boardY < 0 || boardY >= HEIGHT)
            return false;

        uint16_t shifted = pieceRow << 6-x;

        // over horizontal boundaries
        if ((shifted & FULL_MASK) != shifted)
            return false;

        // collision
        if (shifted & rows[boardY])
            return false;
    }
    return true;
}


// placement of a piece
void Board::place(uint16_t mask, int x, int y) {
    for (int i = 0; i < 4; i++) {

        uint16_t pieceRow = (mask >> (i * 4)) & 0xF;
        if (pieceRow == 0)
            continue;

        int boardY = y + i;
        uint16_t shifted = pieceRow << 6-x;

        rows[boardY] |= shifted;
    }
}


// deleting full rows
int Board::clearFullRows() {
    int cleared = 0;

    for (int y=0; y<HEIGHT; y++) {
        if ((rows[y] & FULL_MASK) == FULL_MASK) {
            for (int i=y; i< HEIGHT - 1; i++) {
                rows[i] = rows[i+1];
            }
            rows[HEIGHT - 1] = 0;
            y--;        // test the same row again
            ++cleared;
        }
    }
    return cleared;
}
