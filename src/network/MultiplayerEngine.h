#pragma once

#include <memory>
#include <string>
#include "NetworkSession.h"
#include "Message.h"
#include "../model/GameState.h"
#include "../model/MultiplayerMode.h"

enum class InputAction {
    MOVE_LEFT,
    MOVE_RIGHT,
    ROTATE_CW,
    ROTATE_CCW,
    SOFT_DROP,
    HARD_DROP
};

class MultiplayerEngine {
public:
    enum class MultiplayerGameMode { RACE, MALUS };
    enum class EngineState { IDLE, LOBBY, GAME_RUNNING, GAME_OVER };

    MultiplayerEngine(const std::string& playerName);
    ~MultiplayerEngine();

    bool startHosting(unsigned short port, MultiplayerGameMode mode);
    std::string getHostInfo() const;
    std::string getPublicHostInfo() const;
    unsigned short getListeningPort() const;
    void setGameMode(MultiplayerGameMode mode);
    void setTargetLines(int lines);
    void enableScreenShare(bool enabled);
    
    bool connectToHost(const std::string& serverIP, unsigned short port, MultiplayerGameMode mode);
    
    void update(float deltaTime);
    void sendInput(InputAction action);
    void sendMalus(MalusType malusType, uint32_t durationMs = 5000);
    
    GameState& getLocalGameState() { return m_localGameState; }
    GameState& getRemoteGameState() { return m_remoteGameState; }
    const Board& getRemoteBoard() const;
    
    EngineState getEngineState() const { return m_engineState; }
    bool isGameRunning() const { return m_engineState == EngineState::GAME_RUNNING; }
    bool isGameOver() const { return m_engineState == EngineState::GAME_OVER; }
    
    MultiplayerGameMode getGameMode() const { return m_gameMode; }
    int getTargetLines() const { return m_targetLines; }
    bool isScreenShareEnabled() const { return m_enableScreenShare; }
    
    ::MultiplayerGameMode* getMultiplayerMode() { return m_multiplayerMode.get(); }
    
    std::string getRemotePlayerName() const;
    uint32_t getLatency() const;
    bool isConnected() const;
    
    int getWinnerId() const { return m_winnerId; }
    std::string getWinnerName() const;
    int checkVictory();
    
    void disconnect();

private:
    std::string m_localPlayerName;
    std::string m_remotePlayerName;
    std::unique_ptr<NetworkSession> m_networkSession;
    GameState m_localGameState;
    GameState m_remoteGameState;
    std::unique_ptr<::MultiplayerGameMode> m_multiplayerMode;
    MultiplayerGameMode m_gameMode;
    int m_targetLines;
    bool m_enableScreenShare;
    EngineState m_engineState;
    int m_winnerId;
    uint32_t m_frameNumber;
    uint8_t m_receiveBuffer[1024];
    
    void processNetworkMessages();
    void sendGameState();
    void broadcastGameState();
    void handleRemoteInput(const Message& msg);
    void handleGameStateUpdate(const Message& msg);
    void handleMalusMessage(const Message& msg);
    void applyRemoteGameState(const GameState& remote);
    void applyInputToLocalState(InputAction action);
};
