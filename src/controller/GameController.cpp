#include "GameController.h"
#include "../model/LevelBasedMode.h"
#include "../model/DeathrunMode.h"
#include "../model/AIMode.h"
#include "../network/MultiplayerEngine.h"
#include "../network/NetworkManager.h"
#include "../network/NetworkConfig.h"
#include "../ai/SimpleAI.h"
#include "../ai/AdvancedAI.h"

// GameController: The main controller that manages the entire game
// It handles input, updates game state, and coordinates between model and view

// Initialize all game state variables
GameController::GameController()
    : m_inputTimer(0.0f),                    // Timer for input delay
      m_multiplayerInputTimer(0.0f),         // Timer for multiplayer input throttling
      m_currentMenuState(MenuState::MAIN_MENU),  // Start at main menu
      m_selectedOption(0),                   // First option selected
      m_shouldExit(false),                   // Don't exit yet
      m_isHosting(false),                    // Not hosting a game
      m_serverIPInput(""),                   // No IP entered yet
      m_localPlayerAI(false),                // Local player is human by default
      m_localPlayerAIAdvanced(false),        // AI difficulty
      m_remotePlayerAI(false),               // Remote player is human by default
      m_remotePlayerAIAdvanced(false),       // Remote AI difficulty
      m_localAIMode(false),                  // Not in local AI mode
      m_aiMoveTimer(0.0f),                   // Timer for AI moves
      m_remoteAIMoveTimer(0.0f) {}          // Timer for remote AI moves

// Clean up when controller is destroyed
GameController::~GameController() {
    disconnectNetwork();  // Make sure we disconnect from network
}

// Handle all events from SFML (key presses, mouse clicks, etc.)
// Note: Key repeat is disabled in main.cpp to prevent duplicate key press events
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
            if (m_localAIMode) {
                m_localAIMode = false;
                setLocalPlayerAI(false, false);
                setRemotePlayerAI(false, false);
            }
            m_currentMenuState = MenuState::MODE_SELECTION;
            m_selectedOption = 0;
        }
        
        // Check if AI is controlling the player - if so, disable input
        bool isAIControlling = false;
        if (m_localAIMode && m_localPlayerAI) {
            // Local AI mode with AI player enabled
            isAIControlling = true;
        } else {
            // Check if game mode is AIMode (AI plays automatically)
            const auto* aiMode = dynamic_cast<const AIMode*>(m_gameState.getGameMode());
            if (aiMode) {
                isAIControlling = true;
            }
        }
        
        // Network mode: send inputs to multiplayer engine (only if game is still running)
        if (m_multiplayerEngine && m_multiplayerEngine->isConnected() && 
            m_multiplayerEngine->isGameRunning() && !m_localAIMode && !isAIControlling) {
            // For movement keys (left/right/down), use InputHandler to track state
            // and send input through the throttled update loop instead of immediately
            // This prevents sending too many inputs when keys are held down
            if (keyPressed->code == sf::Keyboard::Key::Left ||
                keyPressed->code == sf::Keyboard::Key::Right ||
                keyPressed->code == sf::Keyboard::Key::Down) {
                // Track key state for continuous input
                m_inputHandler.handleKeyPress(keyPressed->code);
                // Process first press immediately (no delay), subsequent holds will be throttled
                if (m_inputHandler.wasKeyJustPressed(keyPressed->code)) {
                    if (keyPressed->code == sf::Keyboard::Key::Left) {
                        m_multiplayerEngine->sendInput(InputAction::MOVE_LEFT);
                    } else if (keyPressed->code == sf::Keyboard::Key::Right) {
                        m_multiplayerEngine->sendInput(InputAction::MOVE_RIGHT);
                    } else if (keyPressed->code == sf::Keyboard::Key::Down) {
                        m_multiplayerEngine->sendInput(InputAction::SOFT_DROP);
                    }
                    m_inputHandler.markKeyProcessed(keyPressed->code);
                    // Reset timer so held keys are throttled
                    m_multiplayerInputTimer = 0.0f;
                }
            } else {
                // Discrete actions (rotation, hard drop) - send immediately
                // Key repeat is disabled, so each key press is unique
                if (keyPressed->code == sf::Keyboard::Key::Space) {
                    m_multiplayerEngine->sendInput(InputAction::HARD_DROP);
                } else if (keyPressed->code == sf::Keyboard::Key::Z) {
                    m_multiplayerEngine->sendInput(InputAction::ROTATE_CCW);
                } else if (keyPressed->code == sf::Keyboard::Key::X) {
                    m_multiplayerEngine->sendInput(InputAction::ROTATE_CW);
                } else if (keyPressed->code == sf::Keyboard::Key::Up) {
                    m_multiplayerEngine->sendInput(InputAction::ROTATE_CW);
                }
            }
        } else if (!isAIControlling) {
            // Only process input if AI is not controlling
            if (m_localAIMode && !m_localPlayerAI) {
                // Local AI mode but local player is human - normal input handling
                m_inputHandler.handleKeyPress(keyPressed->code);
            } else if (!m_localAIMode) {
                // Solo mode: normal input handling
                m_inputHandler.handleKeyPress(keyPressed->code);
            }
        }
        // If isAIControlling is true, input is ignored (AI handles everything)
    }
    
    if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
        // Check if AI is controlling - if so, ignore input
        bool isAIControlling = false;
        if (m_localAIMode && m_localPlayerAI) {
            isAIControlling = true;
        } else {
            const auto* aiMode = dynamic_cast<const AIMode*>(m_gameState.getGameMode());
            if (aiMode) {
                isAIControlling = true;
            }
        }
        
        // Always update InputHandler for key releases (needed for multiplayer movement tracking)
        // This ensures movement keys stop being sent when released
        if (!isAIControlling) {
            m_inputHandler.handleKeyRelease(keyReleased->code);
        }
    }
}

// Handle keyboard input when in a menu
void GameController::handleMenuInput(const sf::Keyboard::Key& key) {
    // Get how many options are in the current menu
    int optionCount = m_menuView.getOptionCount(m_currentMenuState, isNetworkMode());
    
    // Special handling for IP input menu (can navigate between input field and Back button)
    if (m_currentMenuState == MenuState::ENTER_IP) {
        if (key == sf::Keyboard::Key::Up || key == sf::Keyboard::Key::Down) {
            // Move selection up or down, wrapping around
            m_selectedOption = (m_selectedOption + (key == sf::Keyboard::Key::Down ? 1 : -1) + optionCount) % optionCount;
        }
        return;
    }
    
    // Handle arrow keys to navigate menu
    if (key == sf::Keyboard::Key::Up) {
        // Move selection up (wrap to bottom if at top)
        m_selectedOption = (m_selectedOption - 1 + optionCount) % optionCount;
    } else if (key == sf::Keyboard::Key::Down) {
        // Move selection down (wrap to top if at bottom)
        m_selectedOption = (m_selectedOption + 1) % optionCount;
    } else if (key == sf::Keyboard::Key::Enter) {
        // Enter key selects the current option
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
                // Host Game - start a server and wait for client
                startHosting(53000);
                m_currentMenuState = MenuState::HOST_GAME;
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // Join Game - go to IP input screen
                m_currentMenuState = MenuState::JOIN_GAME;
                m_selectedOption = 0;
            } else if (m_selectedOption == 2) {
                // AI vs AI (Local) - two AIs play locally, no network needed
                m_localAIMode = true;
                m_gameState.reset();
                m_remoteGameState.reset();
                m_gameState.setGameMode(std::make_unique<LevelBasedMode>());
                m_remoteGameState.setGameMode(std::make_unique<LevelBasedMode>());
                // Local player: Advanced AI, Remote player: Simple AI
                setLocalPlayerAI(true, true);
                setRemotePlayerAI(true, false);
                m_currentMenuState = MenuState::NONE;  // Start playing immediately
                m_selectedOption = 0;
            } else if (m_selectedOption == 3) {
                // Player vs AI - you play, AI plays against you (no network)
                m_localAIMode = true;
                m_gameState.reset();
                m_remoteGameState.reset();
                m_gameState.setGameMode(std::make_unique<LevelBasedMode>());
                m_remoteGameState.setGameMode(std::make_unique<LevelBasedMode>());
                // Local player: Human (you), Remote player: Advanced AI
                setLocalPlayerAI(false, false);
                setRemotePlayerAI(true, true);
                m_currentMenuState = MenuState::NONE;  // Start playing immediately
                m_selectedOption = 0;
            } else if (m_selectedOption == 4) {
                // Back - return to main menu
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 1; // Return to Multiplayer option
            }
        } else if (m_currentMenuState == MenuState::JOIN_GAME) {
            if (key == sf::Keyboard::Key::Escape) {
                m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                m_selectedOption = 1;
            } else if (m_selectedOption == 0) {
                // Local Network - go to IP input
                m_serverIPInput.clear();
                m_currentMenuState = MenuState::ENTER_IP;
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // Internet - go to IP input
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

    // Local AI mode (AI vs AI or Player vs AI, no network needed)
    if (m_localAIMode) {
        // Update both game states
        m_gameState.update(deltaTime);
        m_remoteGameState.update(deltaTime);
        
        // Handle local player (either AI or human)
        updateLocalPlayerInAIMode(deltaTime);
        
        // Handle remote player (AI)
        updateRemotePlayerInAIMode(deltaTime);
        
        // Check if game is over
        if (m_gameState.isGameOver() || m_remoteGameState.isGameOver()) {
            m_currentMenuState = MenuState::GAME_OVER;
            m_selectedOption = 0;
        }
        
        return;
    }

    // Network mode: Update multiplayer engine
    if (m_multiplayerEngine && m_multiplayerEngine->isConnected()) {
        // Update network and game logic
        m_multiplayerEngine->update(deltaTime);
        
        // Get game states from multiplayer engine
        const GameState& localState = m_multiplayerEngine->getLocalGameState();
        const GameState& remoteState = m_multiplayerEngine->getRemoteGameState();
        
        // Sync our local game states with the engine's states
        m_gameState.syncBoard(localState.board());
        m_remoteGameState.syncBoard(remoteState.board());
        
        // Process multiplayer input with throttling (exactly like single player)
        // This prevents sending too many movement commands when keys are held
        if (m_multiplayerEngine->isGameRunning() && !m_localAIMode) {
            m_multiplayerInputTimer += deltaTime;
            if (m_multiplayerInputTimer >= INPUT_DELAY) {
                m_multiplayerInputTimer = 0.0f;
                
                // Send movement inputs if keys are held (throttled, same delay as single player)
                if (m_inputHandler.isLeftPressed()) {
                    m_multiplayerEngine->sendInput(InputAction::MOVE_LEFT);
                }
                if (m_inputHandler.isRightPressed()) {
                    m_multiplayerEngine->sendInput(InputAction::MOVE_RIGHT);
                }
                if (m_inputHandler.isDownPressed()) {
                    m_multiplayerEngine->sendInput(InputAction::SOFT_DROP);
                }
            }
        }
        
        // If local player is AI, make AI moves (only if game is still running)
        if (m_localPlayerAI && m_aiPlayer && m_multiplayerEngine->isGameRunning()) {
            auto& config = NetworkConfig::getInstance();
            m_aiMoveTimer += deltaTime;
            
            // Check if it's time for AI to make a move
            if (m_aiMoveTimer >= config.getAIMoveDelay() && 
                !localState.isClearingLines() && 
                !localState.isGameOver()) {
                
                m_aiMoveTimer = 0.0f;
                
                // Ask AI what move to make
                auto [rotation, column] = m_aiPlayer->chooseMove(localState);
                
                // Send rotation commands
                int currentX = localState.pieceX();
                for (int i = 0; i < rotation; ++i) {
                    m_multiplayerEngine->sendInput(InputAction::ROTATE_CW);
                }
                
                // Send movement commands
                if (column < currentX) {
                    // Move left
                    for (int i = 0; i < currentX - column; ++i) {
                        m_multiplayerEngine->sendInput(InputAction::MOVE_LEFT);
                    }
                } else if (column > currentX) {
                    // Move right
                    for (int i = 0; i < column - currentX; ++i) {
                        m_multiplayerEngine->sendInput(InputAction::MOVE_RIGHT);
                    }
                }
                
                // Drop the piece
                m_multiplayerEngine->sendInput(InputAction::HARD_DROP);
            }
        }
        
        // Check if someone won
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
        
        // Check if AI is controlling the player - if so, skip input processing
        bool isAIControlling = false;
        const auto* aiMode = dynamic_cast<const AIMode*>(m_gameState.getGameMode());
        if (aiMode) {
            // Game mode is AIMode - AI plays automatically
            isAIControlling = true;
        }
        
        // Only process input if AI is not controlling
        if (!isAIControlling) {
            // Check for immediate key presses (first press, no delay)
            // This ensures rapid key presses are never missed
            if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Left)) {
                m_gameState.moveLeft();
                m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Left);
                m_inputTimer = 0.0f;  // Reset timer so held keys are throttled
            } else if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Right)) {
                m_gameState.moveRight();
                m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Right);
                m_inputTimer = 0.0f;  // Reset timer so held keys are throttled
            } else if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Down)) {
                m_gameState.softDrop();
                m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Down);
                m_inputTimer = 0.0f;  // Reset timer so held keys are throttled
            }
            
            // Process input with delay for held keys (throttled movement)
            m_inputTimer += deltaTime;
            if (m_inputTimer >= INPUT_DELAY) {
                m_inputTimer = 0.0f;
                processContinuousInput();
            }
            
            // Process discrete input (rotation - no delay)
            processDiscreteInput();
        }
    }
}

// Process input that can be held down (continuous input)
// This is called with a delay to prevent moving too fast
void GameController::processContinuousInput() {
    // Handle left/right movement (can hold key down)
    if (m_inputHandler.isLeftPressed()) {
        m_gameState.moveLeft();
    }
    
    if (m_inputHandler.isRightPressed()) {
        m_gameState.moveRight();
    }
    
    // Handle soft drop (down arrow - makes piece fall faster)
    if (m_inputHandler.isDownPressed()) {
        m_gameState.softDrop();
    }
}

// Process input that happens once per key press (discrete input)
// These actions happen immediately when you press the key
void GameController::processDiscreteInput() {
    // Handle rotation (happens immediately when you press the key)
    if (m_inputHandler.isRotateClockwisePressed()) {
        m_gameState.rotateClockwise();
        m_inputHandler.resetRotateFlags();  // Reset so it doesn't rotate again
    }
    
    if (m_inputHandler.isRotateCounterClockwisePressed()) {
        m_gameState.rotateCounterClockwise();
        m_inputHandler.resetRotateFlags();  // Reset so it doesn't rotate again
    }
    
    // Handle hard drop (happens immediately when you press space)
    if (m_inputHandler.isHardDropPressed()) {
        m_gameState.hardDrop();
        m_inputHandler.resetHardDropFlag();  // Reset so it doesn't drop again
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

// Start hosting a multiplayer game
void GameController::startHosting(unsigned short port) {
    // Use default port from config if not specified
    if (port == 0) {
        port = NetworkConfig::getInstance().getPort();
    }
    
    // Create multiplayer engine if we don't have one
    if (!m_multiplayerEngine) {
        m_multiplayerEngine = std::make_unique<MultiplayerEngine>("Host");
    }
    
    // Load settings from config
    auto& config = NetworkConfig::getInstance();
    m_multiplayerEngine->setTargetLines(config.getDefaultTargetLines());
    m_multiplayerEngine->enableScreenShare(config.isScreenShareEnabledRace());
    
    // Try to start hosting
    if (m_multiplayerEngine->startHosting(port, MultiplayerEngine::MultiplayerGameMode::RACE)) {
        m_isHosting = true;
        m_gameState.reset();
        // If AI mode was requested, enable it now
        if (m_localPlayerAI && !m_aiPlayer) {
            setLocalPlayerAI(true, m_localPlayerAIAdvanced);
        }
    }
}

// Connect to a server as a client
void GameController::connectToServer(const std::string& serverIP, unsigned short port) {
    // Create multiplayer engine if we don't have one
    if (!m_multiplayerEngine) {
        m_multiplayerEngine = std::make_unique<MultiplayerEngine>("Client");
    }
    
    // Use default port from config if not specified
    if (port == 0) {
        port = NetworkConfig::getInstance().getPort();
    }
    
    // Load settings from config
    auto& config = NetworkConfig::getInstance();
    m_multiplayerEngine->setTargetLines(config.getDefaultTargetLines());
    m_multiplayerEngine->enableScreenShare(config.isScreenShareEnabledRace());
    
    // Try to connect
    if (m_multiplayerEngine->connectToHost(serverIP, port, MultiplayerEngine::MultiplayerGameMode::RACE)) {
        // Connection successful!
        m_isHosting = false;
        m_gameState.reset();
        // If AI mode was requested, enable it now
        if (m_localPlayerAI && !m_aiPlayer) {
            setLocalPlayerAI(true, m_localPlayerAIAdvanced);
        }
    } else {
        // Connection failed - go back to IP input screen
        m_currentMenuState = MenuState::ENTER_IP;
        m_selectedOption = 0;
    }
}

// Disconnect from multiplayer
void GameController::disconnectNetwork() {
    if (m_multiplayerEngine) {
        m_multiplayerEngine->disconnect();
        m_multiplayerEngine.reset();  // Delete the multiplayer engine
    }
    m_isHosting = false;
}

// Check if we're hosting a game
bool GameController::isHosting() const {
    return m_isHosting && m_multiplayerEngine != nullptr;
}

// Check if we're connected as a client
bool GameController::isConnected() const {
    return !m_isHosting && m_multiplayerEngine != nullptr && m_multiplayerEngine->isConnected();
}

// Check if a client is connected (for host)
bool GameController::isClientConnected() const {
    if (m_isHosting && m_multiplayerEngine) {
        return m_multiplayerEngine->isConnected();
    }
    return false;
}

// Check if we're in network mode (either multiplayer or local AI mode)
bool GameController::isNetworkMode() const {
    // Local AI mode counts as network mode for rendering purposes
    if (m_localAIMode) return true;
    // Otherwise check if we're actually connected
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

// Add a character to the IP address input field
// Used when typing an IP address to connect
void GameController::appendToIPInput(char c) {
    // Limit input length to prevent overflow
    if (m_serverIPInput.length() < 25) {
        m_serverIPInput += c;
    }
}

// Remove the last character from the IP address input field (backspace)
void GameController::removeLastIPChar() {
    if (!m_serverIPInput.empty()) {
        m_serverIPInput.pop_back();  // Remove last character
    }
}

// Get the server's local IP address (for same-network play)
// Returns format: "192.168.1.100" (just the IP, no port)
std::string GameController::getServerLocalIP() const {
    if (m_isHosting && m_multiplayerEngine) {
        std::string info = m_multiplayerEngine->getHostInfo();  // Gets "IP:PORT"
        // Extract IP part (everything before the colon)
        size_t colonPos = info.find(':');
        if (colonPos != std::string::npos) {
            return info.substr(0, colonPos);  // Return just the IP
        }
        return info;  // No colon found, return as-is
    }
    return "";
}

// Get the server's public IP address (for internet play)
// Returns format: "123.45.67.89:53000" (IP and port)
std::string GameController::getServerPublicIP() const {
    if (m_isHosting && m_multiplayerEngine) {
        return m_multiplayerEngine->getPublicHostInfo();  // Returns "IP:PORT"
    }
    return "";
}

// Get the last connection error message (if connection failed)
// Useful for showing error messages to the user
std::string GameController::getLastConnectionError() const {
    if (m_multiplayerEngine && m_multiplayerEngine->getNetworkSession()) {
        return m_multiplayerEngine->getNetworkSession()->getLastError();
    }
    return "";
}

// Get network latency in milliseconds (how long messages take to travel)
// Lower is better - shows connection quality
uint32_t GameController::getNetworkLatency() const {
    if (m_multiplayerEngine) {
        return m_multiplayerEngine->getLatency();
    }
    return 0;
}

// Get the name of the remote player (in multiplayer)
std::string GameController::getRemotePlayerName() const {
    if (m_multiplayerEngine) {
        return m_multiplayerEngine->getRemotePlayerName();
    }
    return "";
}

// Enable or disable AI for the local player
void GameController::setLocalPlayerAI(bool enabled, bool useAdvanced) {
    m_localPlayerAI = enabled;
    m_localPlayerAIAdvanced = useAdvanced;
    m_aiMoveTimer = 0.0f;
    
    if (enabled) {
        // Create the AI player (Advanced or Simple)
        m_aiPlayer = useAdvanced 
            ? std::make_unique<AdvancedAI>()
            : std::make_unique<SimpleAI>();
    } else {
        // Remove AI player
        m_aiPlayer.reset();
    }
}

// Enable or disable AI for the remote player (in local AI mode)
void GameController::setRemotePlayerAI(bool enabled, bool useAdvanced) {
    m_remotePlayerAI = enabled;
    m_remotePlayerAIAdvanced = useAdvanced;
    m_remoteAIMoveTimer = 0.0f;
    
    if (enabled) {
        // Create the AI player (Advanced or Simple)
        m_remoteAIPlayer = useAdvanced 
            ? std::make_unique<AdvancedAI>()
            : std::make_unique<SimpleAI>();
    } else {
        // Remove AI player
        m_remoteAIPlayer.reset();
    }
}

// Helper function: Make an AI move for any game state
void GameController::makeAIMove(GameState& gameState, AIPlayer* aiPlayer, float& moveTimer) {
    if (!aiPlayer) {
        return;
    }
    
    // Get config settings
    auto& config = NetworkConfig::getInstance();
    
    // Check if enough time has passed and game is ready
    if (moveTimer >= config.getAIMoveDelay() && 
        !gameState.isClearingLines() && 
        !gameState.isGameOver()) {
        
        moveTimer = 0.0f;  // Reset timer
        
        // Ask AI what move to make (rotation and column)
        auto [rotation, column] = aiPlayer->chooseMove(gameState);
        
        // Apply the rotation
        for (int i = 0; i < rotation; ++i) {
            gameState.rotateClockwise();
        }
        
        // Move to the target column
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
        
        // Drop the piece
        gameState.hardDrop();
    }
}

// Update local player in AI mode (either AI or human input)
void GameController::updateLocalPlayerInAIMode(float deltaTime) {
    if (m_localPlayerAI && m_aiPlayer) {
        // Local player is AI - make AI move
        m_aiMoveTimer += deltaTime;
        makeAIMove(m_gameState, m_aiPlayer.get(), m_aiMoveTimer);
    } else {
        // Local player is human - process keyboard input with throttling (same as single-player)
        // Check for immediate key presses (first press, no delay)
        if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Left)) {
            m_gameState.moveLeft();
            m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Left);
            m_inputTimer = 0.0f;  // Reset timer so held keys are throttled
        } else if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Right)) {
            m_gameState.moveRight();
            m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Right);
            m_inputTimer = 0.0f;  // Reset timer so held keys are throttled
        } else if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Down)) {
            m_gameState.softDrop();
            m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Down);
            m_inputTimer = 0.0f;  // Reset timer so held keys are throttled
        }
        
        // Process input with delay for held keys (throttled movement)
        m_inputTimer += deltaTime;
        if (m_inputTimer >= INPUT_DELAY) {
            m_inputTimer = 0.0f;
            processContinuousInput();
        }
        
        // Process discrete input (rotation - no delay)
        processDiscreteInput();
    }
}

// Update remote player in AI mode (always AI)
void GameController::updateRemotePlayerInAIMode(float deltaTime) {
    if (m_remotePlayerAI && m_remoteAIPlayer) {
        // Remote player is AI - make AI move
        m_remoteAIMoveTimer += deltaTime;
        makeAIMove(m_remoteGameState, m_remoteAIPlayer.get(), m_remoteAIMoveTimer);
    }
}

