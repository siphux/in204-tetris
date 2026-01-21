#pragma once
#include "Board.h"
#include "Tetromino.h"
#include "Score.h"
#include "GameMode.h"
#include <vector>
#include <memory>

// Forward declaration
class GameMode;

// GameState is responsible for:
// - current piece
// - board state
// - score
// - piece movement & gravity
// - game mode management
class GameState {
public:
    GameState();
    ~GameState();

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

    // Drop piece instantly to the bottom
    void hardDrop();

    // Lock current piece into board and spawn a new piece
    void lockPiece();

    // Update the clearing animation
    void updateClearingAnimation(float deltaTime);

    void reset(); // Réinitialiser le jeu

    // Game mode management
    void setGameMode(std::unique_ptr<GameMode> mode);
    GameMode* getGameMode() const;

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
    std::unique_ptr<GameMode> m_gameMode;
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
