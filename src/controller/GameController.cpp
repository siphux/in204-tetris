#include "GameController.h"
#include "../model/LevelBasedMode.h"
#include "../model/DeathrunMode.h"
#include "../model/AIMode.h"
#include "../network/MultiplayerEngine.h"
#include "../network/NetworkManager.h"

GameController::GameController()
    : m_inputTimer(0.0f),
      m_currentMenuState(MenuState::MAIN_MENU),
      m_selectedOption(0),
      m_shouldExit(false),
      m_isHosting(false),
      m_serverIPInput("") {}

GameController::~GameController() {
    disconnectNetwork();
}

void GameController::handleEvent(const sf::Event& event) {
    // Handle IP input (text entry) if in ENTER_IP menu
    if (m_currentMenuState == MenuState::ENTER_IP) {
        // Handle key presses first (for Backspace, Delete, etc.)
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            // Handle editing keys directly
            if (keyPressed->code == sf::Keyboard::Key::Backspace) {
                removeLastIPChar();
                return;
            } else if (keyPressed->code == sf::Keyboard::Key::Delete) {
                // Delete key - same as backspace for simplicity
                removeLastIPChar();
                return;
            } else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                if (m_selectedOption == 1) {
                    m_currentMenuState = MenuState::JOIN_GAME;
                    m_selectedOption = 1;
                } else if (!m_serverIPInput.empty()) {
                    std::string ip = m_serverIPInput;
                    unsigned short port = 53000;
                    
                    size_t colonPos = ip.find(':');
                    if (colonPos != std::string::npos) {
                        std::string portStr = ip.substr(colonPos + 1);
                        ip = ip.substr(0, colonPos);
                        try {
                            int portInt = std::stoi(portStr);
                            if (portInt > 0 && portInt <= 65535) {
                                port = static_cast<unsigned short>(portInt);
                            }
                        } catch (...) {
                        }
                    }
                    
                    connectToServer(ip, port);
                }
                return;
            } else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                // Cancel and go back
                m_currentMenuState = MenuState::JOIN_GAME;
                m_selectedOption = 1; // Return to "Enter IP address" option
                return;
            }
            // For other keys, let handleMenuInput handle them (though it shouldn't be needed here)
            handleMenuInput(keyPressed->code);
            return;
        }
        
        // Handle text input (for typing characters)
        if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
            char c = static_cast<char>(textEntered->unicode);
            if (c >= 32 && c <= 126) {
                if ((c >= '0' && c <= '9') || c == '.' || c == ':') {
                    appendToIPInput(c);
                    m_selectedOption = 0;
                }
            }
        }
        return;
    }
    
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
        
        // Network mode: send inputs to multiplayer engine
        if (m_multiplayerEngine && m_multiplayerEngine->isConnected()) {
            // Convert SFML input to InputAction for multiplayer
            if (keyPressed->code == sf::Keyboard::Key::Left) {
                m_multiplayerEngine->sendInput(InputAction::MOVE_LEFT);
            } else if (keyPressed->code == sf::Keyboard::Key::Right) {
                m_multiplayerEngine->sendInput(InputAction::MOVE_RIGHT);
            } else if (keyPressed->code == sf::Keyboard::Key::Down) {
                m_multiplayerEngine->sendInput(InputAction::SOFT_DROP);
            } else if (keyPressed->code == sf::Keyboard::Key::Space) {
                m_multiplayerEngine->sendInput(InputAction::HARD_DROP);
            } else if (keyPressed->code == sf::Keyboard::Key::Z) {
                m_multiplayerEngine->sendInput(InputAction::ROTATE_CCW);
            } else if (keyPressed->code == sf::Keyboard::Key::X) {
                m_multiplayerEngine->sendInput(InputAction::ROTATE_CW);
            }
        } else {
            // Solo mode: normal input handling
            m_inputHandler.handleKeyPress(keyPressed->code);
        }
    }
    
    if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
        if (!isNetworkMode()) {
            m_inputHandler.handleKeyRelease(keyReleased->code);
        }
    }
}

void GameController::handleMenuInput(const sf::Keyboard::Key& key) {
    int optionCount = m_menuView.getOptionCount(m_currentMenuState, isNetworkMode());
    
    if (m_currentMenuState == MenuState::ENTER_IP) {
        if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::Down) {
            m_selectedOption = (m_selectedOption + (key == sf::Keyboard::Key::Down ? 1 : -1) + optionCount) % optionCount;
        }
        return;
    }
    
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
                // Multiplayer - go to multiplayer menu
                m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                m_selectedOption = 0;
            } else if (m_selectedOption == 2) {
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
        } else if (m_currentMenuState == MenuState::MULTIPLAYER_MENU) {
            if (m_selectedOption == 0) {
                // Host Game
                startHosting(53000);
                m_currentMenuState = MenuState::HOST_GAME;
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // Join Game
                m_currentMenuState = MenuState::JOIN_GAME;
                m_selectedOption = 0;
            } else if (m_selectedOption == 2) {
                // Back
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 1; // Return to Multiplayer option
            }
        } else if (m_currentMenuState == MenuState::JOIN_GAME) {
            if (key == sf::Keyboard::Key::Escape) {
                m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                m_selectedOption = 1;
            } else if (m_selectedOption == 0) {
                connectToServer("localhost", 53000);
            } else if (m_selectedOption == 1) {
                m_serverIPInput.clear();
                m_currentMenuState = MenuState::ENTER_IP;
                m_selectedOption = 0;
            } else if (m_selectedOption == 2) {
                m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                m_selectedOption = 1;
            }
        } else if (m_currentMenuState == MenuState::ENTER_IP) {
            if (m_selectedOption == 1) {
                m_currentMenuState = MenuState::JOIN_GAME;
                m_selectedOption = 1;
            } else if (key == sf::Keyboard::Key::Enter) {
                if (!m_serverIPInput.empty()) {
                    std::string ip = m_serverIPInput;
                    unsigned short port = 53000;
                    
                    size_t colonPos = ip.find(':');
                    if (colonPos != std::string::npos) {
                        std::string portStr = ip.substr(colonPos + 1);
                        ip = ip.substr(0, colonPos);
                        try {
                            int portInt = std::stoi(portStr);
                            if (portInt > 0 && portInt <= 65535) {
                                port = static_cast<unsigned short>(portInt);
                            }
                        } catch (...) {
                        }
                    }
                    
                    connectToServer(ip, port);
                }
            } else if (key == sf::Keyboard::Key::Escape) {
                m_currentMenuState = MenuState::JOIN_GAME;
                m_selectedOption = 1;
            }
        } else if (m_currentMenuState == MenuState::HOST_GAME) {
            if (m_selectedOption == 0) {
                // Cancel hosting
                disconnectNetwork();
                m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                m_selectedOption = 0;
            }
            // ESC also cancels (for convenience)
            if (key == sf::Keyboard::Key::Escape) {
                disconnectNetwork();
                m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                m_selectedOption = 0;
            }
        } else if (m_currentMenuState == MenuState::PAUSE_MENU) {
            if (m_selectedOption == 0) {
                // Resume
                m_currentMenuState = MenuState::NONE;
            } else if (m_selectedOption == 1) {
                // In multiplayer: Disconnect, in solo: Main Menu
                if (isNetworkMode()) {
                    // Disconnect and go back to multiplayer menu
                    disconnectNetwork();
                    m_gameState.reset();
                    m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                    m_selectedOption = 0;
                } else {
                    // Solo mode: Main Menu
                    m_gameState.reset();
                    m_currentMenuState = MenuState::MAIN_MENU;
                    m_selectedOption = 0;
                }
            } else if (m_selectedOption == 2 && isNetworkMode()) {
                // Multiplayer mode: Main Menu (second option)
                disconnectNetwork();
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
    // Handle HOST_GAME menu state (check for connection)
    if (m_currentMenuState == MenuState::HOST_GAME) {
        if (m_multiplayerEngine && m_multiplayerEngine->isConnected()) {
            // Client connected, begin playing
            m_currentMenuState = MenuState::NONE;
        }
        // Continue updating engine even in menu
        if (m_isHosting && m_multiplayerEngine) {
            m_multiplayerEngine->update(deltaTime);
        }
        return;
    }
    
    // If we're in a menu, don't update game state
    if (m_currentMenuState != MenuState::NONE) {
        return;
    }

    // Network mode: Update multiplayer engine
    if (m_multiplayerEngine && m_multiplayerEngine->isConnected()) {
        m_multiplayerEngine->update(deltaTime);
        // Sync states from engine
        const GameState& localState = m_multiplayerEngine->getLocalGameState();
        const GameState& remoteState = m_multiplayerEngine->getRemoteGameState();
        m_gameState.syncBoard(localState.board());
        m_remoteGameState.syncBoard(remoteState.board());
        
        // Check victory condition
        int winner = m_multiplayerEngine->checkVictory();
        if (winner != -1) {
            m_currentMenuState = MenuState::GAME_OVER;
            m_selectedOption = 0;
        }
    } else {
        // Solo mode: normal update
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

const GameState& GameController::getRemoteGameState() const {
    return m_remoteGameState;
}

bool GameController::isGameOver() const {
    return m_gameState.isGameOver();
}

void GameController::startHosting(unsigned short port) {
    if (!m_multiplayerEngine) {
        m_multiplayerEngine = std::make_unique<MultiplayerEngine>("Host");
    }
    
    if (m_multiplayerEngine->startHosting(port, MultiplayerEngine::MultiplayerGameMode::RACE)) {
        m_isHosting = true;
        m_gameState.reset();
        // Stay in HOST_GAME menu until client connects
    }
}

void GameController::connectToServer(const std::string& serverIP, unsigned short port) {
    if (!m_multiplayerEngine) {
        m_multiplayerEngine = std::make_unique<MultiplayerEngine>("Client");
    }
    
    if (m_multiplayerEngine->connectToHost(serverIP, port, MultiplayerEngine::MultiplayerGameMode::RACE)) {
        m_isHosting = false;
        m_gameState.reset();
        // Wait for connection to be established
    }
}

void GameController::disconnectNetwork() {
    if (m_multiplayerEngine) {
        m_multiplayerEngine->disconnect();
        m_multiplayerEngine.reset();
    }
    m_isHosting = false;
}

bool GameController::isHosting() const {
    return m_isHosting && m_multiplayerEngine != nullptr;
}

bool GameController::isConnected() const {
    return !m_isHosting && m_multiplayerEngine != nullptr && m_multiplayerEngine->isConnected();
}

bool GameController::isClientConnected() const {
    if (m_isHosting && m_multiplayerEngine) {
        return m_multiplayerEngine->isConnected();
    }
    return false;
}

bool GameController::isNetworkMode() const {
    return m_multiplayerEngine != nullptr && m_multiplayerEngine->isConnected();
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

void GameController::appendToIPInput(char c) {
    if (m_serverIPInput.length() < 25) {
        m_serverIPInput += c;
    }
}

void GameController::removeLastIPChar() {
    if (!m_serverIPInput.empty()) {
        m_serverIPInput.pop_back();
    }
}

std::string GameController::getServerLocalIP() const {
    if (m_isHosting && m_multiplayerEngine) {
        std::string info = m_multiplayerEngine->getHostInfo();
        // Extract IP part (before the colon)
        size_t colonPos = info.find(':');
        if (colonPos != std::string::npos) {
            return info.substr(0, colonPos);
        }
        return info;
    }
    return "";
}

std::string GameController::getServerPublicIP() const {
    if (m_isHosting && m_multiplayerEngine) {
        return m_multiplayerEngine->getPublicHostInfo();
    }
    return "";
}
