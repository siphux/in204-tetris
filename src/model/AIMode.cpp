#include "AIMode.h"
#include "GameState.h"

AIMode::AIMode() 
    : m_ai(std::make_unique<SimpleAI>()),
      m_moveTimer(0.0f),
      m_linesCleared(0) {}

void AIMode::update(float deltaTime, GameState& gameState) {
    // Don't make moves during line clearing animation or if game is over
    if (gameState.isClearingLines() || gameState.isGameOver()) {
        return;
    }
    
    m_moveTimer += deltaTime;
    
    // Make a move every MOVE_DELAY seconds
    if (m_moveTimer >= MOVE_DELAY) {
        m_moveTimer = 0.0f;
        
        // Get the AI's decision for the current piece
        auto [rotation, column] = m_ai->chooseMove(gameState);
        
        // Apply rotations
        for (int i = 0; i < rotation; ++i) {
            gameState.rotateClockwise();
        }
        
        // Move piece to the target column
        int currentX = gameState.pieceX();
        if (column < currentX) {
            // Move left
            for (int i = 0; i < currentX - column; ++i) {
                gameState.moveLeft();
            }
        } else if (column > currentX) {
            // Move right
            for (int i = 0; i < column - currentX; ++i) {
                gameState.moveRight();
            }
        }
        
        // Hard drop the piece
        gameState.hardDrop();
    }
}

float AIMode::getFallSpeed() const {
    return BASE_SPEED;
}

void AIMode::onLinesClear(int linesCleared, GameState& gameState) {
    m_linesCleared += linesCleared;
}

void AIMode::reset() {
    m_moveTimer = 0.0f;
    m_linesCleared = 0;
    m_ai = std::make_unique<SimpleAI>();
}

const char* AIMode::getModeName() const {
    return "AI Mode";
}

int AIMode::getLinesCleared() const {
    return m_linesCleared;
}
