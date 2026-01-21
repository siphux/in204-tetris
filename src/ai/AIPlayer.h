#ifndef AI_PLAYER_H
#define AI_PLAYER_H
#include <utility>
#include "../model/GameState.h"


// Generic AI Player class
class AIPlayer {
    public:
        virtual ~AIPlayer() = default;

        virtual std::pair<int, int> chooseMove(const GameState& state) = 0;

    protected:
        virtual double boardEvaluation(const Board& board) const = 0;

        Board simulateMove(const Board& board, const Tetromino& piece, int rotation, int column) const;
    };


#endif