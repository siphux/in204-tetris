#pragma once
#include "../model/GameState.h"
#include "InputHandler.h"
#include "../view/MenuView.h"
#include "../view/MusicManager.h"
#include "../ai/AIPlayer.h"
#include "../network/NetworkManager.h"
#include <SFML/Window/Event.hpp>
#include <memory>

class GameMode;
class MultiplayerGameMode;

//Game controller handles game loop, and manages the coordination between input, game state and rendering.
class GameController {
public:
    GameController();
    ~GameController();
    void handleEvent(const sf::Event& event);
    void update(float deltaTime);
    
    const GameState& getGameState() const;
    GameState& getGameState();
    
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
    bool getLocalPlayerReady() const { return m_localPlayerReady; }
    bool getRemotePlayerReady() const { return m_remotePlayerReady; }
    
    // LAN Network methods
    void startHosting(unsigned short port = 53000);
    void connectToHost(const std::string& ip, unsigned short port = 53000);
    void disconnectNetwork();
    bool isNetworkConnected() const;
    std::string getLocalIP() const;
    std::string getIPInput() const { return m_ipInput; }
    
    // Volume control
    float getMusicVolume() const { return m_musicVolume; }
    void setMusicVolume(float volume);
    
    // Local AI mode
    void setLocalPlayerAI(bool enabled, bool useAdvanced = true);
    bool isLocalPlayerAI() const { return m_localPlayerAI; }
    void setRemotePlayerAI(bool enabled, bool useAdvanced = true);  // "Remote" = second/opponent player in local mode
    bool isRemotePlayerAI() const { return m_remotePlayerAI; }
    
    // Network helper methods
    PacketData gameStateToPacket(const GameState& state);
    void packetToGameState(const PacketData& packet, GameState& state);

private:
    GameState m_gameState;
    GameState m_remoteGameState;  // opponent's state in multiplayer
    InputHandler m_inputHandler;
    MenuView m_menuView;
    MenuState m_currentMenuState;
    int m_selectedOption;
    bool m_shouldExit;
    float m_musicVolume;
    
    // Music manager
    std::unique_ptr<MusicManager> m_musicManager;
    
    // Network multiplayer
    std::unique_ptr<NetworkManager> m_networkManager;
    bool m_networkMode;  // LAN network multiplayer mode
    float m_networkUpdateTimer;
    static constexpr float NETWORK_UPDATE_INTERVAL = 0.016f; // ~60 updates/sec
    std::string m_ipInput;  // For JOIN_GAME menu: IP address input
    bool m_localPlayerReady;  // Local player ready status for network games
    bool m_remotePlayerReady;  // Remote player ready status for network games
    
    // AI for multiplayer and local modes
    bool m_localPlayerAI;
    bool m_localPlayerAIAdvanced;
    bool m_remotePlayerAI;
    bool m_remotePlayerAIAdvanced;
    bool m_localAIMode;
    float m_aiMoveTimer;
    float m_remoteAIMoveTimer;
    std::unique_ptr<AIPlayer> m_aiPlayer;
    std::unique_ptr<AIPlayer> m_remoteAIPlayer;
    

    //Track the current game mode for the play again option
    enum class LocalMultiplayerMode {
        NONE,
        AI_VS_AI,
        PLAYER_VS_AI
    } m_currentLocalMultiplayerMode;
    
    // 
    enum class SingleplayerMode {
        NONE,
        LEVEL_MODE,
        SIMPLE_AI,
        ADVANCED_AI
    } m_currentSingleplayerMode;
    
    //track winner
    int m_localAIModeWinnerId;
    std::string m_localAIModeWinnerName;
    
    std::unique_ptr<MultiplayerGameMode> m_multiplayerMode;

    bool m_leftHeld;
    bool m_rightHeld;
    bool m_downHeld;
    float m_leftHoldTimer;
    float m_rightHoldTimer;
    float m_downHoldTimer;
    static constexpr float MOVE_REPEAT_INTERVAL = 0.1f;
    static constexpr float SOFT_DROP_REPEAT_INTERVAL = 0.05f;
    
    // Process continuous input (hold arrow keys)
    void processContinuousInput(float deltaTime);
    
    void updateLocalPlayerInAIMode(float deltaTime);
    void updateRemotePlayerInAIMode(float deltaTime);
    void makeAIMove(GameState& gameState, AIPlayer* aiPlayer, float& moveTimer);
    
    // Process discrete input (rotations, hard drop)
    void processDiscreteInput();
    
    // Handle menu input
    void handleMenuInput(const sf::Keyboard::Key& key);
};
