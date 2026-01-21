#include "AIPlayer.h"

Board AIPlayer::simulateMove(const Board& board, const Tetromino& piece, int rotation, int column) const{
    Board simulatedBoard = board;
    Tetromino rotatedPiece = piece;
    for (int i = 0; i < rotation; ++i){
        rotatedPiece.rotateClockwise();
    }

    const std::vector<Point>& blocks = rotatedPiece.getBlocks();
    
    int row = 0;
    while (!simulatedBoard.checkCollision(blocks, column, row)){
        row++;
    }
    row--;
    for (const auto& block : blocks){
        int x = column + block.x;
        int y = row + block.y;
        if (simulatedBoard.isInside(x, y)) simulatedBoard.setCell(x, y, rotatedPiece.getColorId());
    }
    return simulatedBoard;
}
