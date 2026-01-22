#include "GameController.h"
#include "../model/LevelBasedMode.h"
#include "../model/DeathrunMode.h"
#include "../model/AIMode.h"

GameController::GameController()
    : m_inputTimer(0.0f),
      m_currentMenuState(MenuState::MAIN_MENU),
      m_selectedOption(0),
      m_shouldExit(false) {}

void GameController::handleEvent(const sf::Event& event) {
    // Handle menu input first
    if (m_currentMenuState != MenuState::NONE) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            handleMenuInput(keyPressed->code);
        }
        return;
    }

    // Handle key press/release events for game
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        // Check for pause
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            m_currentMenuState = MenuState::PAUSE_MENU;
            m_selectedOption = 0;
            return;
        }
        
        if (keyPressed->code == sf::Keyboard::Key::R && m_gameState.isGameOver()) {
            m_currentMenuState = MenuState::MODE_SELECTION;
            m_selectedOption = 0;
        }
        m_inputHandler.handleKeyPress(keyPressed->code);
    }
    
    if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
        m_inputHandler.handleKeyRelease(keyReleased->code);
    }
}

void GameController::handleMenuInput(const sf::Keyboard::Key& key) {
    int optionCount = m_menuView.getOptionCount(m_currentMenuState);
    
    // Handle menu navigation
    if (key == sf::Keyboard::Key::Up) {
        m_selectedOption = (m_selectedOption - 1 + optionCount) % optionCount;
    } else if (key == sf::Keyboard::Key::Down) {
        m_selectedOption = (m_selectedOption + 1) % optionCount;
    } else if (key == sf::Keyboard::Key::Enter) {
        // Handle menu selection
        if (m_currentMenuState == MenuState::MAIN_MENU) {
            if (m_selectedOption == 0) {
                // Start Game - go to mode selection
                m_currentMenuState = MenuState::MODE_SELECTION;
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // Exit
                m_shouldExit = true;
            }
        } else if (m_currentMenuState == MenuState::MODE_SELECTION) {
            if (m_selectedOption == 0) {
                // Level Mode
                m_gameState.setGameMode(std::make_unique<LevelBasedMode>());
                m_currentMenuState = MenuState::NONE;
            } else if (m_selectedOption == 1) {
                // Deathrun Mode
                m_gameState.setGameMode(std::make_unique<DeathrunMode>());
                m_currentMenuState = MenuState::NONE;
            } else if (m_selectedOption == 2) {
                // AI Mode - go to AI selection submenu
                m_currentMenuState = MenuState::AI_SELECTION;
                m_selectedOption = 0;
            } else if (m_selectedOption == 3) {
                // Back
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 0;
            }
        } else if (m_currentMenuState == MenuState::AI_SELECTION) {
            if (m_selectedOption == 0) {
                // Simple AI
                m_gameState.setGameMode(std::make_unique<AIMode>(false));
                m_currentMenuState = MenuState::NONE;
            } else if (m_selectedOption == 1) {
                // Advanced AI
                m_gameState.setGameMode(std::make_unique<AIMode>(true));
                m_currentMenuState = MenuState::NONE;
            } else if (m_selectedOption == 2) {
                // Back
                m_currentMenuState = MenuState::MODE_SELECTION;
                m_selectedOption = 2; // Return to AI Mode option
            }
        } else if (m_currentMenuState == MenuState::PAUSE_MENU) {
            if (m_selectedOption == 0) {
                // Resume
                m_currentMenuState = MenuState::NONE;
            } else if (m_selectedOption == 1) {
                // Main Menu
                m_gameState.reset();
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 0;
            }
        } else if (m_currentMenuState == MenuState::GAME_OVER) {
            if (m_selectedOption == 0) {
                // Play Again
                m_currentMenuState = MenuState::MODE_SELECTION;
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // Main Menu
                m_gameState.reset();
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 0;
            }
        }
    } else if (key == sf::Keyboard::Key::Escape && m_currentMenuState == MenuState::PAUSE_MENU) {
        // Resume game with Escape
        m_currentMenuState = MenuState::NONE;
    }
}

void GameController::update(float deltaTime) {
    // If we're in a menu, don't update game state
    if (m_currentMenuState != MenuState::NONE) {
        return;
    }

    // Check if game over and transition to game over menu
    if (m_gameState.isGameOver() && m_currentMenuState == MenuState::NONE) {
        m_currentMenuState = MenuState::GAME_OVER;
        m_selectedOption = 0;
        return;
    }

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
    
    // Handle hard drop (instant, no delay)
    if (m_inputHandler.isHardDropPressed()) {
        m_gameState.hardDrop();
        m_inputHandler.resetHardDropFlag();
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

MenuState GameController::getMenuState() const {
    return m_currentMenuState;
}

void GameController::setMenuState(MenuState state) {
    m_currentMenuState = state;
    m_selectedOption = 0;
}

int GameController::getSelectedOption() const {
    return m_selectedOption;
}

const MenuView& GameController::getMenuView() const {
    return m_menuView;
}
