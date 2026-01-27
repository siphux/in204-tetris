#pragma once
#include "../model/GameState.h"
#include "InputHandler.h"
#include "../view/MenuView.h"
#include "../ai/AIPlayer.h"
#include "../network/NetworkManager.h"
#include <SFML/Window/Event.hpp>
#include <memory>

// Forward declarations
class GameMode;

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
    
    // Get opponent game state (for multiplayer/local AI rendering)
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
    
    // Multiplayer state helpers
    bool isLocalMultiplayerMode() const { return m_localAIMode; }
    bool isNetworkMultiplayerMode() const { return m_networkMode; }
    int getWinnerId() const;
    std::string getWinnerName() const;
    
    // LAN Network methods
    void startHosting(unsigned short port = 53000);
    void connectToHost(const std::string& ip, unsigned short port = 53000);
    void disconnectNetwork();
    bool isNetworkConnected() const;
    std::string getLocalIP() const;
    std::string getIPInput() const { return m_ipInput; }
    
    // Local AI mode: AI vs AI and Player vs AI support
    void setLocalPlayerAI(bool enabled, bool useAdvanced = true);
    bool isLocalPlayerAI() const { return m_localPlayerAI; }
    void setRemotePlayerAI(bool enabled, bool useAdvanced = true);  // "Remote" = second/opponent player in local mode
    bool isRemotePlayerAI() const { return m_remotePlayerAI; }
    
    // Network helper methods
    PacketData gameStateToPacket(const GameState& state);
    void packetToGameState(const PacketData& packet, GameState& state);

private:
    GameState m_gameState;
    GameState m_remoteGameState;  // For multiplayer: opponent's state
    InputHandler m_inputHandler;
    MenuView m_menuView;
    MenuState m_currentMenuState;
    int m_selectedOption;
    bool m_shouldExit;
    
    // Network multiplayer
    std::unique_ptr<NetworkManager> m_networkManager;
    bool m_networkMode;  // LAN network multiplayer mode
    float m_networkUpdateTimer;
    static constexpr float NETWORK_UPDATE_INTERVAL = 0.016f; // ~60 updates/sec
    std::string m_ipInput;  // For JOIN_GAME menu: IP address input
    
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
    
    // Winner tracking for local AI mode
    int m_localAIModeWinnerId;
    std::string m_localAIModeWinnerName;
    
    // Input timing for movement (to prevent too-fast movement)
    float m_inputTimer;
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
