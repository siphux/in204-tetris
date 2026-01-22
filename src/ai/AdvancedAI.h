#ifndef ADVANCED_AI_H
#define ADVANCED_AI_H

#include "SimpleAI.h"

class AdvancedAI : public SimpleAI {
public:
    std::pair<int, int> chooseMove(const GameState& state) override;

private:
    double computeCostWithPosition(const Board& board, const Tetromino& nextPiece, bool recursiveMode) const;
};

#endif
