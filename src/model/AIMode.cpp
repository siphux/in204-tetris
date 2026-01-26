#include "AIMode.h"
#include "GameState.h"
#include "../ai/SimpleAI.h"
#include "../ai/AdvancedAI.h"

AIMode::AIMode(bool useAdvanced) 
    : m_useAdvanced(useAdvanced),
      m_ai(useAdvanced ? static_cast<std::unique_ptr<AIPlayer>>(std::make_unique<AdvancedAI>()) 
                        : static_cast<std::unique_ptr<AIPlayer>>(std::make_unique<SimpleAI>())),
      m_moveTimer(0.0f),
      m_level(),
      m_totalLinesCleared(0) {}

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
    int level = m_level.current();
    float speed = BASE_SPEED - (SPEED_MULTIPLIER * level);
    // Ensure minimum speed
    return speed > 0.05f ? speed : 0.05f;
}

void AIMode::onLinesClear(int linesCleared, GameState& gameState) {
    // Add cleared lines to level tracker
    m_level.addLines(linesCleared);
    // Track total lines cleared
    m_totalLinesCleared += linesCleared;
}

void AIMode::reset() {
    m_moveTimer = 0.0f;
    m_level = Level();
    m_totalLinesCleared = 0;
    // Preserve the AI type choice when resetting
    m_ai = m_useAdvanced ? static_cast<std::unique_ptr<AIPlayer>>(std::make_unique<AdvancedAI>()) 
                         : static_cast<std::unique_ptr<AIPlayer>>(std::make_unique<SimpleAI>());
}

const char* AIMode::getModeName() const {
    return "AI Mode";
}

int AIMode::getLinesCleared() const {
    return m_totalLinesCleared;
}

int AIMode::getCurrentLevel() const {
    return m_level.current();
}
