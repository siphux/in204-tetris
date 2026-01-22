#ifndef SIMPLEAI_H
#define SIMPLEAI_H

#include "AIPlayer.h"

class SimpleAI : public AIPlayer{
public:
    std::pair<int, int> chooseMove(const GameState& state) override;

protected:
    double boardEvaluation(const Board& board) const override;

private:
    int calculateBumpiness(const Board& board) const;
    int calculateCompleteLines(const Board& board) const;
    int calculateHoles(const Board& board) const;
    int maxHeight(const Board& board) const;
    int countMinHeight(const Board& board) const;
    int countMaxHeight(const Board& board) const;
    double isLine(const Board& board) const;
    int countHoleColumn(const Board& board, int minimumLine = 2) const;
};

#endif