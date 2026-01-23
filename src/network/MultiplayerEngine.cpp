#include "MultiplayerEngine.h"
#include "NetworkManager.h"
#include "InternetConnectivity.h"
#include "../model/LevelBasedMode.h"
#include <sstream>
#include <iostream>

MultiplayerEngine::MultiplayerEngine(const std::string& playerName)
    : m_localPlayerName(playerName),
      m_gameMode(MultiplayerGameMode::RACE),
      m_targetLines(40),
      m_enableScreenShare(false),
      m_engineState(EngineState::IDLE),
      m_winnerId(-1),
      m_frameNumber(0) {
}

MultiplayerEngine::~MultiplayerEngine() {
    disconnect();
}

bool MultiplayerEngine::startHosting(unsigned short port, MultiplayerGameMode mode) {
    if (!m_networkSession) {
        m_networkSession = std::make_unique<NetworkSession>(
            NetworkSession::SessionRole::HOST, m_localPlayerName);
    }

    if (!m_networkSession->startHosting(port)) {
        std::cerr << "Failed to start hosting: " << m_networkSession->getLastError() << std::endl;
        return false;
    }

    m_gameMode = mode;
    m_engineState = EngineState::LOBBY;
    m_localGameState.reset();
    
    m_localGameState.setGameMode(std::make_unique<LevelBasedMode>());
    m_remoteGameState.setGameMode(std::make_unique<LevelBasedMode>());
    
    ::MultiplayerGameMode::Mode multiplayerMode = (mode == MultiplayerGameMode::RACE) 
        ? ::MultiplayerGameMode::Mode::RACE 
        : ::MultiplayerGameMode::Mode::MALUS;
    m_multiplayerMode = std::make_unique<::MultiplayerGameMode>(
        multiplayerMode, m_targetLines, m_enableScreenShare);
    
    return true;
}

std::string MultiplayerEngine::getHostInfo() const {
    if (!m_networkSession) return "";
    
    std::ostringstream oss;
    oss << m_networkSession->getLocalIP() << ":" << m_networkSession->getListeningPort();
    return oss.str();
}

std::string MultiplayerEngine::getPublicHostInfo() const {
    if (!m_networkSession) return "";
    
    InternetConnectivity::startFetchingPublicIP();
    
    return InternetConnectivity::getConnectionInfo(
        m_networkSession->getLocalIP(), 
        m_networkSession->getListeningPort()
    );
}

unsigned short MultiplayerEngine::getListeningPort() const {
    if (!m_networkSession) return 0;
    return m_networkSession->getListeningPort();
}

void MultiplayerEngine::setGameMode(MultiplayerGameMode mode) {
    m_gameMode = mode;
    
    ::MultiplayerGameMode::Mode multiplayerMode = (mode == MultiplayerGameMode::RACE) 
        ? ::MultiplayerGameMode::Mode::RACE 
        : ::MultiplayerGameMode::Mode::MALUS;
    
    m_multiplayerMode = std::make_unique<::MultiplayerGameMode>(
        multiplayerMode, m_targetLines, m_enableScreenShare);
}

void MultiplayerEngine::setTargetLines(int lines) {
    m_targetLines = lines;
}

void MultiplayerEngine::enableScreenShare(bool enabled) {
    m_enableScreenShare = enabled;
}

bool MultiplayerEngine::connectToHost(const std::string& serverIP, unsigned short port, MultiplayerGameMode mode) {
    if (!m_networkSession) {
        m_networkSession = std::make_unique<NetworkSession>(
            NetworkSession::SessionRole::CLIENT, m_localPlayerName);
    }

    if (!m_networkSession->connectToHost(serverIP, port)) {
        std::cerr << "Failed to connect: " << m_networkSession->getLastError() << std::endl;
        return false;
    }

    m_gameMode = mode;
    m_remotePlayerName = m_networkSession->getRemotePlayerName();
    m_engineState = EngineState::LOBBY;
    m_localGameState.reset();
    
    m_localGameState.setGameMode(std::make_unique<LevelBasedMode>());
    m_remoteGameState.setGameMode(std::make_unique<LevelBasedMode>());
    
    ::MultiplayerGameMode::Mode multiplayerMode = (mode == MultiplayerGameMode::RACE) 
        ? ::MultiplayerGameMode::Mode::RACE 
        : ::MultiplayerGameMode::Mode::MALUS;
    m_multiplayerMode = std::make_unique<::MultiplayerGameMode>(
        multiplayerMode, m_targetLines, m_enableScreenShare);
    
    m_engineState = EngineState::GAME_RUNNING;
    
    return true;
}

void MultiplayerEngine::update(float deltaTime) {
    if (!m_networkSession) return;

    m_networkSession->update();

    if (!m_networkSession->isConnected()) {
        m_engineState = EngineState::IDLE;
        return;
    }

    if (m_networkSession->getRole() == NetworkSession::SessionRole::HOST) {
        if (m_engineState == EngineState::LOBBY && m_networkSession->pollIncomingConnection()) {
            m_engineState = EngineState::GAME_RUNNING;
        }
    }

    processNetworkMessages();

    if (m_engineState == EngineState::GAME_RUNNING) {
        m_localGameState.update(deltaTime);
        
        if (m_multiplayerMode) {
            m_multiplayerMode->update(deltaTime, m_localGameState, m_remoteGameState);
        }

        static float timeSinceLastUpdate = 0.0f;
        timeSinceLastUpdate += deltaTime;
        if (timeSinceLastUpdate >= 0.033f) {
            sendGameState();
            timeSinceLastUpdate = 0.0f;
        }

        checkVictory();
    }
}

void MultiplayerEngine::sendInput(InputAction action) {
    if (!m_networkSession || !m_networkSession->isConnected()) {
        return;
    }

    applyInputToLocalState(action);

    MessageType msgType;
    switch (action) {
        case InputAction::MOVE_LEFT:
        case InputAction::MOVE_RIGHT:
            msgType = MessageType::MOVE;
            break;
        case InputAction::ROTATE_CW:
        case InputAction::ROTATE_CCW:
            msgType = MessageType::ROTATE;
            break;
        case InputAction::SOFT_DROP:
        case InputAction::HARD_DROP:
            msgType = MessageType::DROP;
            break;
    }

    Message msg(msgType);
    msg.payload() << static_cast<uint8_t>(action);
    msg.payload() << m_frameNumber;
    
    auto serialized = msg.serialize();
    m_networkSession->sendPacket(serialized.data(), serialized.size());
    
    m_frameNumber++;
}

void MultiplayerEngine::sendMalus(MalusType malusType, uint32_t durationMs) {
    if (!m_networkSession || !m_networkSession->isConnected()) {
        return;
    }

    if (m_gameMode != MultiplayerGameMode::MALUS) {
        return;
    }

    Message msg(MessageType::ATTACK);
    msg.payload() << static_cast<uint8_t>(malusType);
    msg.payload() << durationMs;
    msg.payload() << static_cast<uint8_t>(100);
    
    auto serialized = msg.serialize();
    m_networkSession->sendPacket(serialized.data(), serialized.size());
}

const Board& MultiplayerEngine::getRemoteBoard() const {
    return m_remoteGameState.board();
}

std::string MultiplayerEngine::getRemotePlayerName() const {
    return m_remotePlayerName;
}

uint32_t MultiplayerEngine::getLatency() const {
    if (!m_networkSession) return 0;
    return m_networkSession->getLatencyMs();
}

bool MultiplayerEngine::isConnected() const {
    if (!m_networkSession) return false;
    return m_networkSession->isConnected();
}

std::string MultiplayerEngine::getWinnerName() const {
    if (m_winnerId == 0) return m_localPlayerName;
    if (m_winnerId == 1) return m_remotePlayerName;
    return "Tie";
}

void MultiplayerEngine::disconnect() {
    if (m_networkSession) {
        m_networkSession->disconnect();
    }
    m_engineState = EngineState::IDLE;
}

void MultiplayerEngine::processNetworkMessages() {
    if (!m_networkSession) return;

    uint8_t buffer[1024];
    size_t received = m_networkSession->receivePacket(buffer, sizeof(buffer));
    
    if (received > 0) {
        try {
            Message msg = Message::deserialize(buffer, received);
            
            switch (msg.getType()) {
                case MessageType::MOVE:
                case MessageType::ROTATE:
                case MessageType::DROP:
                    handleRemoteInput(msg);
                    break;
                    
                case MessageType::GAME_STATE:
                    handleGameStateUpdate(msg);
                    break;
                    
                case MessageType::ATTACK:
                    handleMalusMessage(msg);
                    break;
                    
                case MessageType::GAME_OVER:
                    m_engineState = EngineState::GAME_OVER;
                    break;
                    
                default:
                    break;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error processing message: " << e.what() << std::endl;
        }
    }
}

void MultiplayerEngine::sendGameState() {
    if (!m_networkSession || !m_networkSession->isConnected()) {
        return;
    }

    Message msg(MessageType::GAME_STATE);
    NetworkManager::serializeGameState(m_localGameState, msg.payload());
    
    auto serialized = msg.serialize();
    m_networkSession->sendPacket(serialized.data(), serialized.size());
}

void MultiplayerEngine::handleRemoteInput(const Message& msg) {
    Message msgCopy = msg;
    uint8_t actionByte;
    uint32_t frameNum;
    
    if (!(msgCopy.payload() >> actionByte >> frameNum)) {
        return;
    }
    
    InputAction action = static_cast<InputAction>(actionByte);
    
    switch (action) {
        case InputAction::MOVE_LEFT:
            m_remoteGameState.moveLeft();
            break;
        case InputAction::MOVE_RIGHT:
            m_remoteGameState.moveRight();
            break;
        case InputAction::ROTATE_CW:
            m_remoteGameState.rotateClockwise();
            break;
        case InputAction::ROTATE_CCW:
            m_remoteGameState.rotateCounterClockwise();
            break;
        case InputAction::SOFT_DROP:
            m_remoteGameState.softDrop();
            break;
        case InputAction::HARD_DROP:
            m_remoteGameState.hardDrop();
            break;
    }
}

void MultiplayerEngine::handleGameStateUpdate(const Message& msg) {
    try {
        Message msgCopy = msg;
        auto serialized = NetworkManager::deserializeGameState(msgCopy.payload());
        m_remoteGameState.syncBoard(serialized.board);
    } catch (...) {
        std::cerr << "Failed to deserialize game state" << std::endl;
    }
}

void MultiplayerEngine::handleMalusMessage(const Message& msg) {
    if (m_gameMode != MultiplayerGameMode::MALUS) return;

    Message msgCopy = msg;
    uint8_t malusType;
    uint32_t duration;
    uint8_t intensity;
    
    if (!(msgCopy.payload() >> malusType >> duration >> intensity)) {
        return;
    }
    
    MalusType type = static_cast<MalusType>(malusType);
    Malus malus{type, duration, intensity};
    
    if (m_multiplayerMode && m_multiplayerMode->getMode() == ::MultiplayerGameMode::Mode::MALUS) {
        int localPlayerId = (m_networkSession->getRole() == NetworkSession::SessionRole::HOST) ? 0 : 1;
        m_multiplayerMode->getMalusMode()->applyMalus(localPlayerId, malus);
        
        switch (type) {
            case MalusType::GARBAGE_LINES:
                break;
            case MalusType::GRAVITY_UP:
                break;
            case MalusType::GRAVITY_DOWN:
                break;
            default:
                break;
        }
    }
}

int MultiplayerEngine::checkVictory() {
    if (!m_multiplayerMode) return -1;

    int winner = m_multiplayerMode->checkVictory(m_localGameState, m_remoteGameState);
    
    if (winner != -1) {
        m_winnerId = winner;
        m_engineState = EngineState::GAME_OVER;
    }
    
    return winner;
}

void MultiplayerEngine::applyRemoteGameState(const GameState& remote) {
    m_remoteGameState.syncBoard(remote.board());
}

void MultiplayerEngine::applyInputToLocalState(InputAction action) {
    switch (action) {
        case InputAction::MOVE_LEFT:
            m_localGameState.moveLeft();
            break;
        case InputAction::MOVE_RIGHT:
            m_localGameState.moveRight();
            break;
        case InputAction::ROTATE_CW:
            m_localGameState.rotateClockwise();
            break;
        case InputAction::ROTATE_CCW:
            m_localGameState.rotateCounterClockwise();
            break;
        case InputAction::SOFT_DROP:
            m_localGameState.softDrop();
            break;
        case InputAction::HARD_DROP:
            m_localGameState.hardDrop();
            break;
    }
}
