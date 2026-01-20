#pragma once
#include "Board.h"
#include "Tetromino.h"
#include "Score.h"
#include "Level.h"

// GameState is responsible for:
// - current piece
// - board state
// - score and level
// - piece movement & gravity
class GameState {
public:
    GameState();

    // Update the game logic (falling)
    void update(float deltaTime);

    // Move piece horizontally
    void moveLeft();
    void moveRight();

    // Drop piece by one row (gravity or soft drop)
    void softDrop();

    // Lock current piece into board and spawn a new piece
    void lockPiece();

    // Accessors for rendering
    const Board& board() const;
    const Tetromino& currentPiece() const;
    const Tetromino& nextPiece() const;
    int pieceX() const;
    int pieceY() const;
    int score() const;
    int level() const;

private:
    Board m_board;
    Tetromino m_currentPiece;
    Tetromino m_nextPiece;
    Score m_score;
    Level m_level;

    int m_x; // current piece position X
    int m_y; // current piece position Y

    float m_fallTimer;

    // Spawn a new piece at the top of the board
    void spawnNewPiece();
};
