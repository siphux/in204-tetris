#pragma once
#include "Board.h"
#include "Tetromino.h"
#include "Score.h"
#include "Level.h"
#include <vector>

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

    // Rotate piece (with wall kicks)
    void rotateClockwise();
    void rotateCounterClockwise();

    // Drop piece by one row (gravity or soft drop)
    void softDrop();

    // Lock current piece into board and spawn a new piece
    void lockPiece();

    // Update the clearing animation
    void updateClearingAnimation(float deltaTime);

    void reset(); // Réinitialiser le jeu

    // Accessors for rendering
    const Board& board() const;
    const Tetromino& currentPiece() const;
    const Tetromino& nextPiece() const;
    int pieceX() const;
    int pieceY() const;
    int getGhostY() const;
    int score() const;
    int level() const;
    bool isGameOver() const;
    
    // Animation state accessors
    bool isClearingLines() const;
    float getClearAnimationProgress() const; // Returns 0.0 to 1.0

private:
    Board m_board;
    Tetromino m_currentPiece;
    Tetromino m_nextPiece;
    Score m_score;
    Level m_level;
    bool m_isClearingLines;
    float m_clearAnimationTimer;
    std::vector<int> m_linesToClear;
    static constexpr float CLEAR_ANIMATION_DURATION = 0.5f;

    bool m_gameOver;

    int m_x; // current piece position X
    int m_y; // current piece position Y


    float m_fallTimer;

    std::vector<TetrominoType> m_pieceBag;
    int m_bagIndex;

    // Spawn a new piece at the top of the board
    void spawnNewPiece();

    void refillBag();
};
