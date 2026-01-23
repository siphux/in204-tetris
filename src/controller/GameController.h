#pragma once
#include "../model/GameState.h"
#include "InputHandler.h"
#include "../view/MenuView.h"
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
    
    // Input timing for movement (to prevent too-fast movement)
    float m_inputTimer;
    static constexpr float INPUT_DELAY = 0.1f; // 100ms delay between moves
    
    // Process continuous input (movement keys)
    void processContinuousInput();
    
    // Process discrete input (rotation)
    void processDiscreteInput();
    
    // Handle menu input
    void handleMenuInput(const sf::Keyboard::Key& key);
};
