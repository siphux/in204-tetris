#include "AIPlayer.h"

Board AIPlayer::simulateMove(const Board& board, const Tetromino& piece, int rotation, int column) const{
    Board simulatedBoard = board;
    Tetromino rotatedPiece = piece;
    for (int i = 0; i < rotation; ++i){
        rotatedPiece.rotateClockwise();
    }

    const std::vector<Point>& blocks = rotatedPiece.getBlocks();

    int row = 0;
    while (row < Board::Height){
        if (simulatedBoard.checkCollision(blocks, column, row + 1)){
            break;
        }
        row++;
    }
    
    for (const auto& block : blocks){
        int x = column + block.x;
        int y = row + block.y;
        if (simulatedBoard.isInside(x, y)) {
            simulatedBoard.setCell(x, y, rotatedPiece.getColorId());
        }
    }
    
    std::vector<int> fullLines;
    for (int y = Board::Height - 1; y >= 0; y--){
        bool full = true;
        for (int x = 0; x < Board::Width; x++){
            if (simulatedBoard.isEmpty(x, y)){
                full = false;
                break;
            }
        }
        if (full){
            fullLines.push_back(y);
        }
    }
    
    if (!fullLines.empty()){
        int writeY = Board::Height - 1;
        for (int readY = Board::Height - 1; readY >= 0; readY--){
            bool isFullLine = false;
            for (int fullY : fullLines){
                if (readY == fullY){
                    isFullLine = true;
                    break;
                }
            }
            if (isFullLine){
                continue;
            }
            
            if (writeY != readY){
                for (int x = 0; x < Board::Width; x++){
                    simulatedBoard.setCell(x, writeY, simulatedBoard.getCell(x, readY));
                }
            }
            writeY--;
        }
        
        while (writeY >= 0){
            for (int x = 0; x < Board::Width; x++){
                simulatedBoard.setCell(x, writeY, 0);
            }
            writeY--;
        }
    }
    
    return simulatedBoard;
}
