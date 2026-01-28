#pragma once
#include "Board.h"
#include "Tetromino.h"
#include "Score.h"
#include "GameMode.h"
#include <vector>
#include <memory>

// Forward declaration
class GameMode;

// GameState manages the overall state of the game with the current and next piece,
// the board, score, piece movement and gravity, gamemode management.

class GameState {
public:
    GameState();
    ~GameState();

    void update(float deltaTime);

    void moveLeft();
    void moveRight();
    void rotateClockwise();
    void rotateCounterClockwise();

    void softDrop();
    void hardDrop();

    void lockPiece();

    void updateClearingAnimation(float deltaTime);

    void reset(); 
    void setGameMode(std::unique_ptr<GameMode> mode);
    GameMode* getGameMode() const;

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
    float getClearAnimationProgress() const;
    
    void addGarbageLines(int numLines);
    
    void syncBoard(const Board& board);
    
    void syncPiecePosition(int x, int y, int rotation);

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

    int m_x;
    int m_y;

    float m_fallTimer;

    std::vector<TetrominoType> m_pieceBag;
    int m_bagIndex;

    void spawnNewPiece();

    void refillBag();
};
