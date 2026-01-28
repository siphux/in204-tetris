#include "SimpleAI.h"
#include <limits>
#include <cmath>

std::pair<int, int> SimpleAI::chooseMove(const GameState& state){
    double bestScore = -std::numeric_limits<double>::infinity(); // we have a maximization problem
    int bestRotation = 0;
    int bestColumn = 0;
    const Board& board = state.board();
    const Tetromino& piece = state.currentPiece();

    for (int rotation = 0; rotation < 4; rotation++){
        Tetromino rotatedPiece = piece;
        for (int r = 0; r < rotation; ++r){
            rotatedPiece.rotateClockwise();
        }
        
        const auto& blocks = rotatedPiece.getBlocks();
        
        int minX = 0, maxX = 0;
        for (const auto& block : blocks) {
            minX = std::min(minX, block.x);
            maxX = std::max(maxX, block.x);
        }
        
        for (int column = -minX; column < Board::Width - maxX; column++){
            bool validPlacement = true;
            for (const auto& block : blocks) {
                int x = column + block.x;
                if (x < 0 || x >= Board::Width) {
                    validPlacement = false;
                    break;
                }
            }
            
            if (!validPlacement) continue;
            
            Board simulatedBoard = simulateMove(board, piece, rotation, column);

            double score = boardEvaluation(simulatedBoard);
            if (score > bestScore){
                bestScore = score;
                bestRotation = rotation;
                bestColumn = column;
            }
        }
    }
    return {bestRotation, bestColumn};
}

double SimpleAI::boardEvaluation(const Board& board) const{
    double nbHole = (double) calculateHoles(board);
    double maxH = (double) countMaxHeight(board);
    double minH = (double) countMinHeight(board);
    double line = isLine(board);
    double holeColumn = (double) countHoleColumn(board, 2);
    double bump = (double) calculateBumpiness(board);
    
    
    double heightPenalty = maxH * maxH * 0.5;
    double heightDiff = maxH - minH;
    
    return line - (nbHole * 10.0) - (heightDiff * 2.0) - (holeColumn * 5.0) - (bump * 1.0) - heightPenalty;
}



int SimpleAI::maxHeight(const Board& board) const{
    double maxHeight = 0;
    for (int x = 0; x < Board::Width; x++){
        for (int y = 0; y < Board::Height; y++){

            if (!board.isEmpty(x, y)){
                if (Board::Height - y > maxHeight){
                    maxHeight = Board::Height - y;
                }
                break;
            }
        }
    }
    return maxHeight;
}


int SimpleAI::calculateBumpiness(const Board& board) const{
    int bumpiness = 0;
    int previousHeight = -1;
    for (int x = 0; x < Board::Width; x++){
        for (int y = 0; y < Board::Height; y++){
            if (previousHeight == -1 && !board.isEmpty(x, y)){
                previousHeight = Board::Height - y;
            }
            else{
            if (!board.isEmpty(x, y)){
                bumpiness += std::abs(previousHeight - (Board::Height - y));
                previousHeight = Board::Height - y;
                break;
            }
        }
    }
}
    return bumpiness;
}


int SimpleAI::calculateCompleteLines(const Board& board) const {
    int completedLines = 0;
    for (int y = 0; y < Board::Height; y++){
        bool isFull = true;
        for (int x = 0; x < Board::Width; x++){
            if (board.isEmpty(x, y)){
                isFull = false;
                break;
            }
        }
        if (isFull){
            completedLines++;
        }
    }
    return completedLines;
}


int SimpleAI::calculateHoles(const Board& board) const{
    int holes = 0;
    for (int x = 0; x < Board::Width; x++){
        bool blockFound = false;
        for (int y = 0 ; y < Board::Height; y++){
            if (!board.isEmpty(x, y)) blockFound = true;
            if (blockFound && board.isEmpty(x, y)) holes++;
        }
    }
    return holes;
}

int SimpleAI::countMinHeight(const Board& board) const {
    int minHeight = Board::Height;
    for (int x = 0; x < Board::Width; x++) {
        for (int y = 0; y < Board::Height; y++) {
            if (!board.isEmpty(x, y)) {
                int height = Board::Height - y;
                if (height < minHeight) {
                    minHeight = height;
                }
                break;
            }
        }
    }
    return minHeight == Board::Height ? 0 : minHeight;
}

int SimpleAI::countMaxHeight(const Board& board) const {
    int maxHeight = 0;
    for (int x = 0; x < Board::Width; x++) {
        for (int y = 0; y < Board::Height; y++) {
            if (!board.isEmpty(x, y)) {
                int height = Board::Height - y;
                if (height > maxHeight) {
                    maxHeight = height;
                }
                break;
            }
        }
    }
    return maxHeight;
}

double SimpleAI::isLine(const Board& board) const {
    int completedLines = 0;
    for (int y = 0; y < Board::Height; y++) {
        bool isFull = true;
        for (int x = 0; x < Board::Width; x++) {
            if (board.isEmpty(x, y)) {
                isFull = false;
                break;
            }
        }
        if (isFull) {
            completedLines++;
        }
    }
    
    if (completedLines == 0) {
        return 0;
    }
    
    double bonus = completedLines * completedLines * 100.0;
    return bonus;
}

int SimpleAI::countHoleColumn(const Board& board, int minimumLine) const {
    int countLongHole = 0;
    for (int x = 0; x < Board::Width; x++) {
        int row = 0;
        for (int y = Board::Height - 1; y >= 0; y--) {
            bool leftBlocked = (x - 1 < 0) || !board.isEmpty(x - 1, y);
            bool rightBlocked = (x + 1 >= Board::Width) || !board.isEmpty(x + 1, y);
            
            if (leftBlocked && rightBlocked && board.isEmpty(x, y)) {
                row++;
            } else if (!board.isEmpty(x, y)) {
                row = 0;
            }
        }
        
        if (row > minimumLine) {
            countLongHole += row;
        }
    }
    return countLongHole;
}