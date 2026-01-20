#include "GameController.h"

GameController::GameController()
    : m_inputTimer(0.0f) {}

void GameController::handleEvent(const sf::Event& event) {
    // Handle key press/release events
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::R && m_gameState.isGameOver()) {
            m_gameState.reset();
        }
        m_inputHandler.handleKeyPress(keyPressed->code);
    }
    
    if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
        m_inputHandler.handleKeyRelease(keyReleased->code);
    }
}

void GameController::update(float deltaTime) {
    // Update game state (piece falling)
    m_gameState.update(deltaTime);
    
    // Process input with delay for movement
    m_inputTimer += deltaTime;
    if (m_inputTimer >= INPUT_DELAY) {
        m_inputTimer = 0.0f;
        processContinuousInput();
    }
    
    // Process discrete input (rotation - no delay)
    processDiscreteInput();
}

void GameController::processContinuousInput() {
    // Handle left/right movement
    if (m_inputHandler.isLeftPressed()) {
        m_gameState.moveLeft();
    }
    
    if (m_inputHandler.isRightPressed()) {
        m_gameState.moveRight();
    }
    
    // Handle soft drop (down arrow)
    if (m_inputHandler.isDownPressed()) {
        m_gameState.softDrop();
    }
}

void GameController::processDiscreteInput() {
    // Handle rotation (instant, no delay)
    if (m_inputHandler.isRotateClockwisePressed()) {
        m_gameState.rotateClockwise();
        m_inputHandler.resetRotateFlags();
    }
    
    if (m_inputHandler.isRotateCounterClockwisePressed()) {
        m_gameState.rotateCounterClockwise();
        m_inputHandler.resetRotateFlags();
    }
}

const GameState& GameController::getGameState() const {
    return m_gameState;
}

GameState& GameController::getGameState() {
    return m_gameState;
}

bool GameController::isGameOver() const {
    return m_gameState.isGameOver();
}
