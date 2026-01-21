#include "SimpleAI.h"
#include <limits>
#include <cmath>

std::pair<int, int> SimpleAI::chooseMove(const GameState& state){
    double bestScore = -std::numeric_limits<double>::infinity();
    int bestRotation = 0;
    int bestColumn = 0;
    const Board& board = state.board();
    const Tetromino& piece = state.currentPiece();

    for (int rotation = 0; rotation < 4; rotation++){
        // Créer une copie de la pièce avec la rotation
        Tetromino rotatedPiece = piece;
        for (int r = 0; r < rotation; ++r){
            rotatedPiece.rotateClockwise();
        }
        
        const auto& blocks = rotatedPiece.getBlocks();
        
        // Calculer les limites valides pour cette rotation
        int minX = 0, maxX = 0;
        for (const auto& block : blocks) {
            minX = std::min(minX, block.x);
            maxX = std::max(maxX, block.x);
        }
        
        // Parcourir uniquement les colonnes valides
        for (int column = -minX; column < Board::Width - maxX; column++){
            // Vérifier que tous les blocs seront dans les limites
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
    double bumpiness = (double) calculateBumpiness(board);
    double holes = (double) calculateHoles(board);
    double linesCleared = (double) calculateCompleteLines(board);
    double max_height = (double) maxHeight(board);
    return -2 * bumpiness - 4 * holes + 5 * linesCleared - 2 * max_height;
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