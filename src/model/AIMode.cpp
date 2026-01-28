#include "AIMode.h"
#include "GameState.h"
#include "../ai/SimpleAI.h"
#include "../ai/AdvancedAI.h"
#include "../ConfigManager.h"

AIMode::AIMode(bool useAdvanced) 
    : m_useAdvanced(useAdvanced),
      m_ai(useAdvanced ? static_cast<std::unique_ptr<AIPlayer>>(std::make_unique<AdvancedAI>()) 
                        : static_cast<std::unique_ptr<AIPlayer>>(std::make_unique<SimpleAI>())),
      m_moveTimer(0.0f),
      m_level(),
      m_totalLinesCleared(0) {}

void AIMode::update(float deltaTime, GameState& gameState) {
    if (gameState.isClearingLines() || gameState.isGameOver()) {
        return;
    }
    
    m_moveTimer += deltaTime;
    
    // Add delay between AI moves
    float moveDelay = ConfigManager::getInstance().getAIMoveDelay();
    if (m_moveTimer >= moveDelay) {
        m_moveTimer = 0.0f;
        
        auto [rotation, column] = m_ai->chooseMove(gameState);
        for (int i = 0; i < rotation; ++i) {
            gameState.rotateClockwise();
        }
        
        int currentX = gameState.pieceX();
        if (column < currentX) {
            for (int i = 0; i < currentX - column; ++i) {
                gameState.moveLeft();
            }
        } else if (column > currentX) {
            for (int i = 0; i < column - currentX; ++i) {
                gameState.moveRight();
            }
        }
        gameState.hardDrop();
    }
}

float AIMode::getFallSpeed() const {
    int level = m_level.current();
    float speed = BASE_SPEED - (SPEED_MULTIPLIER * level);
    //Minimum speed
    return speed > 0.05f ? speed : 0.05f;
}

void AIMode::onLinesClear(int linesCleared, GameState& gameState) {
    m_level.addLines(linesCleared);
    m_totalLinesCleared += linesCleared;
}

void AIMode::reset() {
    m_moveTimer = 0.0f;
    m_level = Level();
    m_totalLinesCleared = 0;
    // Preserve the AI type in case the player wants to play again
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
