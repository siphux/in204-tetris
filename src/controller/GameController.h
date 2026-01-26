#pragma once
#include "../model/GameState.h"
#include "InputHandler.h"
#include "../view/MenuView.h"
#include "../ai/AIPlayer.h"
#include <SFML/Window/Event.hpp>
#include <memory>

// Forward declarations
class GameMode;
class MultiplayerEngine;

// GameController manages the game loop and coordinates between
// input handling, game state updates, and rendering
class GameController {
public:
    GameController();
    ~GameController();
    
    // Process SFML events (call this in your main loop)
    void handleEvent(const sf::Event& event);
    
    // Update game logic (call this every frame with deltaTime)
    void update(float deltaTime);
    
    // Get the current game state (for rendering)
    const GameState& getGameState() const;
    GameState& getGameState();
    
    // Get remote game state (for multiplayer rendering)
    const GameState& getRemoteGameState() const;
    
    // Check if game is over
    bool isGameOver() const;

    // Menu state management
    MenuState getMenuState() const;
    void setMenuState(MenuState state);
    int getSelectedOption() const;
    
    // Get menu view for rendering
    const MenuView& getMenuView() const;
    
    // Check if application should close
    bool shouldExit() const { return m_shouldExit; }
    
    // Network multiplayer methods
    void startHosting(unsigned short port = 53000);
    void connectToServer(const std::string& serverIP, unsigned short port = 53000);
    void disconnectNetwork();
    bool isHosting() const;
    bool isConnected() const;
    bool isClientConnected() const;  // For server: check if client is connected
    bool isNetworkMode() const;
    
    // IP input for joining remote servers
    const std::string& getIPInput() const { return m_serverIPInput; }
    void appendToIPInput(char c);
    void removeLastIPChar();
    
    // Get server's local IP address (for hosting)
    std::string getServerLocalIP() const;
    
    // Get server's public IP address (for internet play)
    std::string getServerPublicIP() const;
    
    // Get last connection error message
    std::string getLastConnectionError() const;
    
    // Get network latency (for multiplayer)
    uint32_t getNetworkLatency() const;
    
    // Get remote player name (for multiplayer)
    std::string getRemotePlayerName() const;
    
    // AI vs AI and Player vs AI support
    void setLocalPlayerAI(bool enabled, bool useAdvanced = true);
    bool isLocalPlayerAI() const { return m_localPlayerAI; }
    void setRemotePlayerAI(bool enabled, bool useAdvanced = true);
    bool isRemotePlayerAI() const { return m_remotePlayerAI; }

private:
    GameState m_gameState;
    GameState m_remoteGameState;  // For multiplayer: opponent's state
    InputHandler m_inputHandler;
    MenuView m_menuView;
    MenuState m_currentMenuState;
    int m_selectedOption;
    bool m_shouldExit;
    
    // Network
    std::unique_ptr<MultiplayerEngine> m_multiplayerEngine;
    bool m_isHosting;  // true = server, false = client or solo
    std::string m_serverIPInput;  // For IP input in ENTER_IP menu
    
    // AI for multiplayer and local modes
    bool m_localPlayerAI;
    bool m_localPlayerAIAdvanced;
    bool m_remotePlayerAI;
    bool m_remotePlayerAIAdvanced;
    bool m_localAIMode;  // Local AI vs AI mode (no network)
    float m_aiMoveTimer;
    float m_remoteAIMoveTimer;
    std::unique_ptr<AIPlayer> m_aiPlayer;
    std::unique_ptr<AIPlayer> m_remoteAIPlayer;
    
    // Input timing for movement (to prevent too-fast movement)
    float m_inputTimer;
    float m_multiplayerInputTimer;  // Separate timer for multiplayer input throttling
    static constexpr float INPUT_DELAY = 0.1f; // 100ms delay between moves (same for single and multiplayer)
    
    // Process continuous input (movement keys)
    void processContinuousInput();
    
    // Helper functions for local AI mode
    void updateLocalPlayerInAIMode(float deltaTime);
    void updateRemotePlayerInAIMode(float deltaTime);
    void makeAIMove(GameState& gameState, AIPlayer* aiPlayer, float& moveTimer);
    
    // Process discrete input (rotation)
    void processDiscreteInput();
    
    // Handle menu input
    void handleMenuInput(const sf::Keyboard::Key& key);
};
