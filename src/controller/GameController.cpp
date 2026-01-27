#include "GameController.h"
#include "../model/LevelBasedMode.h"
#include "../model/AIMode.h"
#include "../ai/SimpleAI.h"
#include "../ai/AdvancedAI.h"
#include <iostream>

namespace {
constexpr int TARGET_LINES = 40;
constexpr float AI_MOVE_DELAY = 0.2f;
}

// GameController: The main controller that manages the entire game
// It handles input, updates game state, and coordinates between model and view

// Initialize all game state variables
GameController::GameController()
        : m_inputTimer(0.0f),                    // Timer for input delay
            m_currentMenuState(MenuState::MAIN_MENU),  // Start at main menu
      m_selectedOption(0),                   // First option selected
      m_shouldExit(false),                   // Don't exit yet
      m_musicVolume(50.0f),                  // Default 50% volume
      m_networkManager(nullptr),             // No network by default
      m_networkMode(false),                  // Not in network mode
      m_networkUpdateTimer(0.0f),            // Network update timer
      m_ipInput(""),                         // Empty IP input
      m_localPlayerReady(false),             // Local player not ready
      m_remotePlayerReady(false),            // Remote player not ready
      m_localPlayerAI(false),                // Local player is human by default
      m_localPlayerAIAdvanced(false),        // AI difficulty
      m_remotePlayerAI(false),               // Second player is human by default
      m_remotePlayerAIAdvanced(false),       // Second player AI difficulty
      m_localAIMode(false),                  // Not in local AI mode
      m_aiMoveTimer(0.0f),                   // Timer for AI moves
      m_remoteAIMoveTimer(0.0f),             // Timer for second player AI moves
      m_localAIModeWinnerId(-1),             // No winner yet
      m_localAIModeWinnerName("") {          // No winner name yet
    // Initialize music manager
    m_musicManager = std::make_unique<MusicManager>();
    if (m_musicManager->initialize()) {
        m_musicManager->play();
    }
}

// Clean up when controller is destroyed
GameController::~GameController() = default;

// Handle all events from SFML (key presses, mouse clicks, etc.)
// Note: Key repeat is disabled in main.cpp to prevent duplicate key press events
void GameController::handleEvent(const sf::Event& event) {
    // Handle menu input first
    if (m_currentMenuState != MenuState::NONE) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            handleMenuInput(keyPressed->code);
        }
        // Handle text input for JOIN_GAME menu
        if (m_currentMenuState == MenuState::JOIN_GAME) {
            if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
                char c = static_cast<char>(textEntered->unicode);
                // Allow digits and dots for IP address
                if ((c >= '0' && c <= '9') || c == '.') {
                    if (m_ipInput.length() < 15) {  // Max IP length
                        m_ipInput += c;
                    }
                }
                // Handle backspace
                else if (c == 8 && !m_ipInput.empty()) {  // Backspace
                    m_ipInput.pop_back();
                }
            }
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
        
        if (!isAIControlling) {
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
        
        // Always update InputHandler for key releases (needed for movement tracking)
        if (!isAIControlling) {
            m_inputHandler.handleKeyRelease(keyReleased->code);
        }
    }
}

// Handle keyboard input when in a menu
void GameController::handleMenuInput(const sf::Keyboard::Key& key) {
    // Special handling for Escape in HOST_GAME menu
    if (m_currentMenuState == MenuState::HOST_GAME && key == sf::Keyboard::Key::Escape) {
        disconnectNetwork();
        m_currentMenuState = MenuState::LAN_MULTIPLAYER;
        m_selectedOption = 0;
        return;
    }
    
    // Handle volume slider left/right in pause and settings menus
    // Must be BEFORE up/down navigation to prevent conflicts
    if ((m_currentMenuState == MenuState::PAUSE_MENU || m_currentMenuState == MenuState::SETTINGS_MENU) && 
        m_selectedOption == 0) {
        if (key == sf::Keyboard::Key::Left) {
            m_musicVolume = std::max(0.0f, m_musicVolume - 5.0f);
            setMusicVolume(m_musicVolume);
            return;  // Don't process as menu navigation
        } else if (key == sf::Keyboard::Key::Right) {
            m_musicVolume = std::min(100.0f, m_musicVolume + 5.0f);
            setMusicVolume(m_musicVolume);
            return;  // Don't process as menu navigation
        }
    }
    
    // Get how many options are in the current menu
    int optionCount = m_menuView.getOptionCount(m_currentMenuState);
    
    // Prevent division by zero if menu has no options
    if (optionCount == 0) {
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
                // Settings
                m_currentMenuState = MenuState::SETTINGS_MENU;
                m_selectedOption = 0;
            } else if (m_selectedOption == 3) {
                // Exit
                m_shouldExit = true;
            }
        } else if (m_currentMenuState == MenuState::SETTINGS_MENU) {
            if (m_selectedOption == 1) {
                // Back
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 0;
            }
        } else if (m_currentMenuState == MenuState::MODE_SELECTION) {
            if (m_selectedOption == 0) {
                // Level Mode
                m_gameState.setGameMode(std::make_unique<LevelBasedMode>());
                m_currentMenuState = MenuState::NONE;
            } else if (m_selectedOption == 1) {
                // AI Mode - go to AI selection submenu
                m_currentMenuState = MenuState::AI_SELECTION;
                m_selectedOption = 0;
            } else if (m_selectedOption == 2) {
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
                m_currentMenuState = MenuState::LOCAL_MULTIPLAYER;
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // LAN Multiplayer
                m_currentMenuState = MenuState::LAN_MULTIPLAYER;
                m_selectedOption = 0;
            } else if (m_selectedOption == 2) {
                // Back - return to main menu
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 1; // Return to Multiplayer option
            }
        } else if (m_currentMenuState == MenuState::LAN_MULTIPLAYER) {
            if (m_selectedOption == 0) {
                // Host Game
                startHosting();
                m_currentMenuState = MenuState::HOST_GAME;
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // Join Game
                m_ipInput = "";  // Clear IP input
                m_currentMenuState = MenuState::JOIN_GAME;
                m_selectedOption = 0;
            } else if (m_selectedOption == 2) {
                // Back
                m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                m_selectedOption = 1;
            }
        } else if (m_currentMenuState == MenuState::HOST_GAME) {
            // No selectable options in HOST_GAME - Escape handled above
        } else if (m_currentMenuState == MenuState::JOIN_GAME) {
            if (m_selectedOption == 0) {
                // Connect
                if (!m_ipInput.empty()) {
                    connectToHost(m_ipInput);
                    // If connection successful, game will start (NONE state)
                    // If failed, stay in JOIN_GAME menu
                }
            } else if (m_selectedOption == 1) {
                // Back
                m_currentMenuState = MenuState::LAN_MULTIPLAYER;
                m_selectedOption = 1;
            }
        } else if (m_currentMenuState == MenuState::NETWORK_READY) {
            if (m_selectedOption == 0) {
                // Ready button
                m_localPlayerReady = !m_localPlayerReady;  // Toggle ready state
            } else if (m_selectedOption == 1) {
                // Back button - disconnect
                m_localPlayerReady = false;
                disconnectNetwork();
                m_currentMenuState = MenuState::LAN_MULTIPLAYER;
                m_selectedOption = 0;
            }
        } else if (m_currentMenuState == MenuState::LOCAL_MULTIPLAYER) {
            if (m_selectedOption == 0) {
                // AI vs AI - two AIs play locally, no network needed
                m_localAIMode = true;
                m_gameState.reset();
                m_remoteGameState.reset();
                m_gameState.setGameMode(std::make_unique<LevelBasedMode>());
                m_remoteGameState.setGameMode(std::make_unique<LevelBasedMode>());
                // Player 1 (left): Advanced AI, Player 2 (right): Simple AI
                setLocalPlayerAI(true, true);
                setRemotePlayerAI(true, false);
                m_currentMenuState = MenuState::NONE;  // Start playing immediately
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // Player vs AI - you play, AI plays against you (no network)
                m_localAIMode = true;
                m_gameState.reset();
                m_remoteGameState.reset();
                m_gameState.setGameMode(std::make_unique<LevelBasedMode>());
                m_remoteGameState.setGameMode(std::make_unique<LevelBasedMode>());
                // Player 1 (you): Human, Player 2 (opponent): Advanced AI
                setLocalPlayerAI(false, false);
                setRemotePlayerAI(true, true);
                m_currentMenuState = MenuState::NONE;  // Start playing immediately
                m_selectedOption = 0;
            } else if (m_selectedOption == 2) {
                // Back - return to multiplayer menu
                m_currentMenuState = MenuState::MULTIPLAYER_MENU;
                m_selectedOption = 0;
            }
        } else if (m_currentMenuState == MenuState::PAUSE_MENU) {
            if (m_selectedOption == 1) {
                // Resume
                m_currentMenuState = MenuState::NONE;
            } else if (m_selectedOption == 2) {
                // Solo mode or local AI mode: Main Menu - reset all AI state
                m_localAIMode = false;
                m_localPlayerAI = false;
                m_remotePlayerAI = false;
                m_aiPlayer.reset();
                m_remoteAIPlayer.reset();
                m_aiMoveTimer = 0.0f;
                m_remoteAIMoveTimer = 0.0f;
                m_gameState.reset();
                m_remoteGameState.reset();
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 0;
            }
        } else if (m_currentMenuState == MenuState::GAME_OVER) {
            if (m_selectedOption == 0) {
                // Play Again - reset all AI state
                m_localAIMode = false;
                m_localPlayerAI = false;
                m_remotePlayerAI = false;
                m_aiPlayer.reset();
                m_remoteAIPlayer.reset();
                m_aiMoveTimer = 0.0f;
                m_remoteAIMoveTimer = 0.0f;
                m_localAIModeWinnerId = -1;
                m_localAIModeWinnerName = "";
                m_gameState.reset();
                m_remoteGameState.reset();
                m_currentMenuState = MenuState::MODE_SELECTION;
                m_selectedOption = 0;
            } else if (m_selectedOption == 1) {
                // Main Menu - reset all AI state
                m_localAIMode = false;
                m_localPlayerAI = false;
                m_remotePlayerAI = false;
                m_aiPlayer.reset();
                m_remoteAIPlayer.reset();
                m_aiMoveTimer = 0.0f;
                m_remoteAIMoveTimer = 0.0f;
                m_localAIModeWinnerId = -1;
                m_localAIModeWinnerName = "";
                m_gameState.reset();
                m_remoteGameState.reset();
                m_currentMenuState = MenuState::MAIN_MENU;
                m_selectedOption = 0;
            }
        }
    } else if (key == sf::Keyboard::Key::Escape && m_currentMenuState == MenuState::PAUSE_MENU) {
        // Resume game with Escape
        m_currentMenuState = MenuState::NONE;
    } else if (key == sf::Keyboard::Key::Escape && m_currentMenuState == MenuState::SETTINGS_MENU) {
        // Back to main menu with Escape
        m_currentMenuState = MenuState::MAIN_MENU;
        m_selectedOption = 0;
    }
}

void GameController::update(float deltaTime) {
    // Update music manager to handle looping
    if (m_musicManager) {
        m_musicManager->update();
        
        // Switch music based on game state
        if (m_currentMenuState == MenuState::NONE) {
            // We're playing - use game music
            m_musicManager->playTrack(MusicTrack::GAME);
        } else {
            // We're in a menu - use menu music
            m_musicManager->playTrack(MusicTrack::MENU);
        }
    }
    
    // If we're in a menu, don't update game state (unless we're networking)
    if (m_currentMenuState != MenuState::NONE && !(m_networkMode && m_networkManager)) {
        return;
    }
    
    // Handle HOST_GAME menu: check if client connected, transition to NETWORK_READY
    if (m_currentMenuState == MenuState::HOST_GAME && m_networkMode && m_networkManager) {
        if (m_networkManager->isConnected()) {
            m_localPlayerReady = false;
            m_remotePlayerReady = false;
            m_currentMenuState = MenuState::NETWORK_READY;
        }
        return;
    }

    // Local AI mode (AI vs AI or Player vs AI, no network needed)
    if (m_localAIMode) {
        // Update both game states
        m_gameState.update(deltaTime);
        m_remoteGameState.update(deltaTime);
        
        // Handle local player (either AI or human)
        updateLocalPlayerInAIMode(deltaTime);
        
        // Handle second player (AI opponent)
        updateRemotePlayerInAIMode(deltaTime);
        
        // Check for marathon victory (first to reach target lines)
        int targetLines = TARGET_LINES;
        int player1Lines = m_gameState.getGameMode() ? m_gameState.getGameMode()->getLinesCleared() : 0;
        int player2Lines = m_remoteGameState.getGameMode() ? m_remoteGameState.getGameMode()->getLinesCleared() : 0;
        
        bool marathonVictory = (player1Lines >= targetLines) || (player2Lines >= targetLines);
        
        // Check if game is over (marathon victory or board filled)
        if (marathonVictory || m_gameState.isGameOver() || m_remoteGameState.isGameOver()) {
            // Determine the winner
            if (player1Lines >= targetLines && player2Lines < targetLines) {
                m_localAIModeWinnerId = 0;
                m_localAIModeWinnerName = "Player 1";
            } else if (player2Lines >= targetLines && player1Lines < targetLines) {
                m_localAIModeWinnerId = 1;
                m_localAIModeWinnerName = "Player 2";
            } else if (m_remoteGameState.isGameOver() && !m_gameState.isGameOver()) {
                m_localAIModeWinnerId = 0;
                m_localAIModeWinnerName = "Player 1";
            } else if (m_gameState.isGameOver() && !m_remoteGameState.isGameOver()) {
                m_localAIModeWinnerId = 1;
                m_localAIModeWinnerName = "Player 2";
            } else {
                m_localAIModeWinnerId = -1;  // Tie or both died
                m_localAIModeWinnerName = "";
            }
            
            m_currentMenuState = MenuState::GAME_OVER;
            m_selectedOption = 0;
        }
        
        return;
    }

    // Network multiplayer mode
    if (m_networkMode && m_networkManager) {
        // Update network (accept connections if hosting)
        m_networkManager->update();
        
        // If we're in NETWORK_READY state, wait for both players to be ready
        if (m_currentMenuState == MenuState::NETWORK_READY) {
            // Sync ready status over network
            m_networkUpdateTimer += deltaTime;
            if (m_networkUpdateTimer >= NETWORK_UPDATE_INTERVAL) {
                m_networkUpdateTimer = 0.0f;
                
                if (m_networkManager->isConnected()) {
                    // Send local ready status (empty game state with just ready flag)
                    PacketData readyPacket = gameStateToPacket(m_gameState);
                    readyPacket.isReady = m_localPlayerReady;
                    m_networkManager->sendGameState(readyPacket);
                    
                    // Receive opponent ready status
                    auto opponentData = m_networkManager->receiveOpponentState();
                    if (opponentData.has_value()) {
                        m_remotePlayerReady = opponentData.value().isReady;
                    }
                }
            }
            
            // Check if both players are ready
            if (m_localPlayerReady && m_remotePlayerReady && m_networkManager->isConnected()) {
                // Start the game
                m_gameState.reset();
                m_remoteGameState.reset();
                m_gameState.setGameMode(std::make_unique<LevelBasedMode>());
                m_remoteGameState.setGameMode(std::make_unique<LevelBasedMode>());
                m_currentMenuState = MenuState::NONE;
                m_selectedOption = 0;
            }
            
            return;
        }
        
        // Update local game state
        m_gameState.update(deltaTime);
        
        // Handle local player input (same flow as solo mode)
        if (!m_gameState.isGameOver()) {
            if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Left)) {
                m_gameState.moveLeft();
                m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Left);
                m_inputTimer = 0.0f;
            } else if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Right)) {
                m_gameState.moveRight();
                m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Right);
                m_inputTimer = 0.0f;
            } else if (m_inputHandler.wasKeyJustPressed(sf::Keyboard::Key::Down)) {
                m_gameState.softDrop();
                m_inputHandler.markKeyProcessed(sf::Keyboard::Key::Down);
                m_inputTimer = 0.0f;
            }

            m_inputTimer += deltaTime;
            if (m_inputTimer >= INPUT_DELAY) {
                m_inputTimer = 0.0f;
                processContinuousInput();
            }

            processDiscreteInput();
        }
        
        // Network sync timer
        m_networkUpdateTimer += deltaTime;
        if (m_networkUpdateTimer >= NETWORK_UPDATE_INTERVAL) {
            m_networkUpdateTimer = 0.0f;
            
            // Send local state to opponent
            if (m_networkManager->isConnected()) {
                PacketData localData = gameStateToPacket(m_gameState);
                m_networkManager->sendGameState(localData);
                
                // Receive opponent state
                auto opponentData = m_networkManager->receiveOpponentState();
                if (opponentData.has_value()) {
                    packetToGameState(opponentData.value(), m_remoteGameState);
                }
            } else {
                // Connection was lost during gameplay
                m_currentMenuState = MenuState::PAUSE_MENU;
                m_selectedOption = 0;
                m_networkMode = false;
                std::cerr << "Connection to opponent lost!" << std::endl;
            }
        }
        
        // Check if either player has lost
        if (m_gameState.isGameOver() || m_remoteGameState.isGameOver()) {
            if (m_remoteGameState.isGameOver() && !m_gameState.isGameOver()) {
                m_localAIModeWinnerId = 0;
                m_localAIModeWinnerName = "You";
            } else if (m_gameState.isGameOver() && !m_remoteGameState.isGameOver()) {
                m_localAIModeWinnerId = 1;
                m_localAIModeWinnerName = "Opponent";
            } else {
                m_localAIModeWinnerId = -1;
                m_localAIModeWinnerName = "";
            }
            m_currentMenuState = MenuState::GAME_OVER;
            m_selectedOption = 0;
        }
        
        return;
    }

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

int GameController::getWinnerId() const {
    return m_localAIMode ? m_localAIModeWinnerId : -1;
}

std::string GameController::getWinnerName() const {
    return m_localAIMode ? m_localAIModeWinnerName : "";
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

// Enable or disable AI for the second player (in local AI mode)
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
    
    // Check if enough time has passed and game is ready
    if (moveTimer >= AI_MOVE_DELAY && 
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

// Update second player in AI mode (always AI)
void GameController::updateRemotePlayerInAIMode(float deltaTime) {
    if (m_remotePlayerAI && m_remoteAIPlayer) {
        // Second player is AI - make AI move
        m_remoteAIMoveTimer += deltaTime;
        makeAIMove(m_remoteGameState, m_remoteAIPlayer.get(), m_remoteAIMoveTimer);
    }
}

// Network helper: Convert GameState to PacketData for transmission
PacketData GameController::gameStateToPacket(const GameState& state) {
    PacketData packet;
    
    // Copy grid data from board
    const Board& board = state.board();
    for (int y = 0; y < 21; ++y) {
        for (int x = 0; x < 10; ++x) {
            packet.grid[y][x] = static_cast<int>(board.getCell(x, y));
        }
    }
    
    // Current piece info
    packet.currentPieceType = static_cast<int>(state.currentPiece().getType());
    packet.currentPieceX = state.pieceX();
    packet.currentPieceY = state.pieceY();
    packet.currentPieceRotation = static_cast<int>(state.currentPiece().getRotationState());
    
    // Game stats (no next piece in packet - not needed for opponent view)
    packet.score = state.score();
    packet.level = state.level();
    packet.isGameOver = state.isGameOver();
    
    return packet;
}

// Network helper: Apply PacketData to GameState
void GameController::packetToGameState(const PacketData& packet, GameState& state) {
    // Sync the board by creating a temporary board with received grid data
    Board tempBoard;
    for (int y = 0; y < 21; ++y) {
        for (int x = 0; x < 10; ++x) {
            tempBoard.setCell(x, y, packet.grid[y][x]);
        }
    }
    state.syncBoard(tempBoard);
    
    // Sync opponent's falling piece position
    state.syncPiecePosition(packet.currentPieceX, packet.currentPieceY, packet.currentPieceRotation);
}

// LAN Network methods
void GameController::startHosting(unsigned short port) {
    if (!m_networkManager) {
        m_networkManager = std::make_unique<NetworkManager>();
    }
    
    if (m_networkManager->host(port)) {
        m_networkMode = true;
        m_localAIMode = false;
        m_localPlayerReady = false;
        m_remotePlayerReady = false;
        m_currentMenuState = MenuState::HOST_GAME;  // Show waiting for connection
    }
}

void GameController::connectToHost(const std::string& ip, unsigned short port) {
    if (!m_networkManager) {
        m_networkManager = std::make_unique<NetworkManager>();
    }
    
    if (m_networkManager->connect(ip, port)) {
        m_networkMode = true;
        m_localAIMode = false;
        m_localPlayerReady = false;
        m_remotePlayerReady = false;
        m_currentMenuState = MenuState::NETWORK_READY;  // Go to ready menu
    }
}

void GameController::disconnectNetwork() {
    if (m_networkManager) {
        m_networkManager->disconnect();
    }
    m_networkMode = false;
}

bool GameController::isNetworkConnected() const {
    return m_networkMode && m_networkManager && m_networkManager->isConnected();
}

std::string GameController::getLocalIP() const {
    return NetworkManager::getLocalIP();
}

void GameController::setMusicVolume(float volume) {
    m_musicVolume = std::max(0.0f, std::min(100.0f, volume));
    if (m_musicManager) {
        m_musicManager->setVolume(m_musicVolume);
    }
}
