#include "AdvancedAI.h"
#include <limits>
#include <algorithm>

std::pair<int, int> AdvancedAI::chooseMove(const GameState& state){
    double bestScore = std::numeric_limits<double>::infinity();
    int bestRotation = 0;
    int bestColumn = 0;
    const Board& board = state.board();
    const Tetromino& piece = state.currentPiece();
    const Tetromino& nextPiece = state.nextPiece();

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
            
            double score = computeCostWithPosition(simulatedBoard, nextPiece, true);
            
            if (score < bestScore){
                bestScore = score;
                bestRotation = rotation;
                bestColumn = column;
            }
        }
    }
    return {bestRotation, bestColumn};
}

double AdvancedAI::computeCostWithPosition(const Board& board, const Tetromino& nextPiece, bool recursiveMode) const {
    double firstCost = -boardEvaluation(board);
    
    if (recursiveMode) {
        double minNextCost = std::numeric_limits<double>::infinity();
        
        for (int rotation = 0; rotation < 4; rotation++){
            Tetromino rotatedNextPiece = nextPiece;
            for (int r = 0; r < rotation; ++r){
                rotatedNextPiece.rotateClockwise();
            }
            
            const auto& blocks = rotatedNextPiece.getBlocks();
            
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
                
                Board nextSimulatedBoard = simulateMove(board, nextPiece, rotation, column);
                double nextCost = -boardEvaluation(nextSimulatedBoard);
                
                minNextCost = std::min(minNextCost, nextCost);
            }
        }
        
        return firstCost + (minNextCost == std::numeric_limits<double>::infinity() ? 0 : minNextCost * 0.5);
    } else {
        return firstCost;
    }
}


