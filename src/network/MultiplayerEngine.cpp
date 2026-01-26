#include "MultiplayerEngine.h"
#include "NetworkManager.h"
#include "InternetConnectivity.h"
#include "../model/LevelBasedMode.h"
#include "../model/Board.h"
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

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
    // Create network session if needed
    if (!m_networkSession) {
        m_networkSession = std::make_shared<NetworkSession>(
            NetworkSession::SessionRole::HOST, m_localPlayerName);
    }

    // Start hosting
    if (!m_networkSession->startHosting(port)) {
        std::cerr << "[HOST] Failed to start hosting: " << m_networkSession->getLastError() << std::endl;
        return false;
    }
    std::cerr << "[HOST] Server started successfully. Waiting for client..." << std::endl;

    // Create and start network thread
    if (!m_networkThread) {
        m_networkThread = std::make_unique<NetworkThread>();
        m_networkThread->setNetworkSession(m_networkSession);
        m_networkThread->start();
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

// Connect to a host server as a client
bool MultiplayerEngine::connectToHost(const std::string& serverIP, unsigned short port, MultiplayerGameMode mode) {
    // Create network session if we don't have one yet
    if (!m_networkSession) {
        m_networkSession = std::make_shared<NetworkSession>(
            NetworkSession::SessionRole::CLIENT, m_localPlayerName);
    }
    
    // Store reconnect info so we can try again if connection drops
    m_networkSession->setReconnectInfo(serverIP, port);
    
    // Try to connect to the server
    if (!m_networkSession->connectToHost(serverIP, port)) {
        std::cerr << "Failed to connect: " << m_networkSession->getLastError() << std::endl;
        return false;
    }

    // Connection successful! Set up the game
    m_gameMode = mode;
    m_remotePlayerName = m_networkSession->getRemotePlayerName();
    m_engineState = EngineState::LOBBY;
    
    // Reset both game states
    m_localGameState.reset();
    m_localGameState.setGameMode(std::make_unique<LevelBasedMode>());
    m_remoteGameState.setGameMode(std::make_unique<LevelBasedMode>());
    
    // Set up the multiplayer mode (Race or Malus)
    ::MultiplayerGameMode::Mode multiplayerMode = (mode == MultiplayerGameMode::RACE) 
        ? ::MultiplayerGameMode::Mode::RACE 
        : ::MultiplayerGameMode::Mode::MALUS;
    m_multiplayerMode = std::make_unique<::MultiplayerGameMode>(
        multiplayerMode, m_targetLines, m_enableScreenShare);
    
    // Start the game
    m_engineState = EngineState::GAME_RUNNING;
    
    return true;
}

// Update multiplayer engine every frame
void MultiplayerEngine::update(float deltaTime) {
    if (!m_networkSession) return;

    // Update network session (check connection, heartbeat, etc.)
    m_networkSession->update();

    // Check if we're still connected
    if (!m_networkSession->isConnected()) {
        // If we're still trying to connect, wait
        if (m_networkSession->getStatus() == NetworkSession::ConnectionStatus::CONNECTING) {
            return;
        }
        // Connection lost - go back to lobby or idle
        if (m_engineState == EngineState::GAME_RUNNING) {
            m_engineState = EngineState::LOBBY;
        } else {
            m_engineState = EngineState::IDLE;
        }
        return;
    }
    
    // If we just connected, start the game
    if (m_engineState == EngineState::IDLE && m_networkSession->isConnected()) {
        m_engineState = EngineState::GAME_RUNNING;
    }

    // If we're the host, check for incoming connections
    if (m_networkSession->getRole() == NetworkSession::SessionRole::HOST) {
        if (m_engineState == EngineState::LOBBY && m_networkSession->pollIncomingConnection()) {
            // Client connected! Start the game
            std::cerr << "[HOST] Client connected! Starting game..." << std::endl;
            m_engineState = EngineState::GAME_RUNNING;
        }
    }

    processNetworkMessages();

    if (m_engineState == EngineState::GAME_RUNNING) {
        // Check for victory BEFORE updating (so we stop immediately when target is reached)
        int winner = checkVictory();
        
        // If someone won, stop updating and notify the other player
        if (winner != -1) {
            // Send GAME_OVER message to notify the other player
            Message gameOverMsg(MessageType::GAME_OVER);
            gameOverMsg.setPlayerId(winner);
            if (m_networkThread && m_networkThread->isRunning()) {
                m_networkThread->queueSend(gameOverMsg);
            } else if (m_networkSession && m_networkSession->isConnected()) {
                auto serialized = gameOverMsg.serialize();
                m_networkSession->sendPacket(serialized.data(), serialized.size());
            }
            // Game state is now GAME_OVER, so updates will stop on next frame
            return;
        }
        
        // Only update if game is still running (not over)
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
    }
}

void MultiplayerEngine::sendInput(InputAction action) {
    if (!m_networkSession || !m_networkSession->isConnected()) {
        return;
    }
    
    // Don't accept input if game is over
    if (m_engineState == EngineState::GAME_OVER) {
        return;
    }

    applyInputToLocalState(action);

    InputBufferEntry entry;
    entry.action = action;
    entry.frameNumber = m_frameNumber;
    entry.timestamp = 0.0f;
    
    m_inputBuffer.push_back(entry);
    if (m_inputBuffer.size() > MAX_BUFFER_SIZE) {
        m_inputBuffer.erase(m_inputBuffer.begin());
    }

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
    
    // Queue message to be sent by network thread (thread-safe)
    if (m_networkThread && m_networkThread->isRunning()) {
        m_networkThread->queueSend(msg);
    } else if (m_networkSession && m_networkSession->isConnected()) {
        // Fallback to direct send if thread not available
        auto serialized = msg.serialize();
        m_networkSession->sendPacket(serialized.data(), serialized.size());
    }
    
    m_frameNumber++;
}

void MultiplayerEngine::sendMalus(MalusType malusType, uint32_t durationMs) {
    if (!m_networkSession || !m_networkSession->isConnected()) {
        return;
    }
    
    // Don't send malus if game is over
    if (m_engineState == EngineState::GAME_OVER) {
        return;
    }

    if (m_gameMode != MultiplayerGameMode::MALUS) {
        return;
    }

    Message msg(MessageType::ATTACK);
    msg.payload() << static_cast<uint8_t>(malusType);
    msg.payload() << durationMs;
    msg.payload() << static_cast<uint8_t>(100);
    
    // Queue message to be sent by network thread (thread-safe)
    if (m_networkThread && m_networkThread->isRunning()) {
        m_networkThread->queueSend(msg);
    } else if (m_networkSession && m_networkSession->isConnected()) {
        // Fallback to direct send if thread not available
        auto serialized = msg.serialize();
        m_networkSession->sendPacket(serialized.data(), serialized.size());
    }
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

// Disconnect from the multiplayer session
void MultiplayerEngine::disconnect() {
    // Stop network thread first
    if (m_networkThread) {
        m_networkThread->stop();
        m_networkThread.reset();
    }
    
    if (m_networkSession) {
        m_networkSession->disconnect();
    }
    // Reset to idle state
    m_engineState = EngineState::IDLE;
}

// Process messages received from the network
void MultiplayerEngine::processNetworkMessages() {
    // Get messages from network thread (thread-safe)
    if (m_networkThread && m_networkThread->isRunning()) {
        // Process all received messages
        while (m_networkThread->hasReceivedMessages()) {
            Message msg = m_networkThread->popReceivedMessage();
            
            // Handle different message types
            switch (msg.getType()) {
                case MessageType::JOIN:
                    // Player joined
                    break;
                case MessageType::JOIN_ACK:
                    // Join acknowledged
                    break;
                case MessageType::START:
                    // Game start
                    if (m_engineState == EngineState::LOBBY) {
                        m_engineState = EngineState::GAME_RUNNING;
                    }
                    break;
                case MessageType::MOVE:
                case MessageType::ROTATE:
                case MessageType::DROP:
                    // Remote player input
                    handleRemoteInput(msg);
                    break;
                case MessageType::GAME_STATE:
                    // Full game state update
                    handleGameStateUpdate(msg);
                    break;
                case MessageType::ATTACK:
                    // Malus/attack received
                    handleMalusMessage(msg);
                    break;
                case MessageType::GAME_OVER:
                    // Game ended
                    m_engineState = EngineState::GAME_OVER;
                    break;
                case MessageType::DISCONNECT:
                    // Player disconnected
                    disconnect();
                    break;
                case MessageType::LINES_CLEARED:
                    // Lines cleared notification
                    break;
            }
        }
    } else if (m_networkSession) {
        // Fallback to direct receive if thread not available
        uint8_t buffer[1024];
        size_t received = m_networkSession->receivePacket(buffer, sizeof(buffer));
        
        if (received > 0) {
            try {
                Message msg = Message::deserialize(buffer, received);
                
                // Handle different message types
                switch (msg.getType()) {
                    case MessageType::MOVE:
                    case MessageType::ROTATE:
                    case MessageType::DROP:
                        // Remote player made a move - apply it
                        handleRemoteInput(msg);
                        break;
                        
                    case MessageType::GAME_STATE:
                        // Remote player sent their game state - update ours
                        handleGameStateUpdate(msg);
                        break;
                        
                    case MessageType::ATTACK:
                        // Remote player attacked us with a malus
                        handleMalusMessage(msg);
                        break;
                        
                    case MessageType::GAME_OVER:
                        // Game ended
                        m_engineState = EngineState::GAME_OVER;
                        break;
                        
                    default:
                        // Unknown message type - ignore it
                        break;
                }
            } catch (const std::exception& e) {
                // Something went wrong parsing the message
                std::cerr << "Error processing message: " << e.what() << std::endl;
            }
        }
    }
}

void MultiplayerEngine::sendGameState() {
    if (!m_networkSession || !m_networkSession->isConnected()) {
        return;
    }

    Message msg(MessageType::GAME_STATE);
    NetworkManager::serializeGameState(m_localGameState, msg.payload());
    
    // Queue message to be sent by network thread (thread-safe)
    if (m_networkThread && m_networkThread->isRunning()) {
        m_networkThread->queueSend(msg);
    } else if (m_networkSession && m_networkSession->isConnected()) {
        // Fallback to direct send if thread not available
        auto serialized = msg.serialize();
        m_networkSession->sendPacket(serialized.data(), serialized.size());
    }
}

bool MultiplayerEngine::validateInput(InputAction action, const GameState& state) {
    const Tetromino& piece = state.currentPiece();
    int baseX = state.pieceX();
    int baseY = state.pieceY();
    
    switch (action) {
        case InputAction::MOVE_LEFT:
            for (const auto& offset : piece.getBlocks()) {
                int x = baseX + offset.x - 1;
                int y = baseY + offset.y;
                if (x < 0 || (y >= 1 && y < Board::Height && state.board().getCell(x, y) != 0)) {
                    return false;
                }
            }
            return true;
        case InputAction::MOVE_RIGHT:
            for (const auto& offset : piece.getBlocks()) {
                int x = baseX + offset.x + 1;
                int y = baseY + offset.y;
                if (x >= Board::Width || (y >= 1 && y < Board::Height && state.board().getCell(x, y) != 0)) {
                    return false;
                }
            }
            return true;
        case InputAction::ROTATE_CW:
        case InputAction::ROTATE_CCW:
        case InputAction::SOFT_DROP:
        case InputAction::HARD_DROP:
            return true;
        default:
            return false;
    }
}

void MultiplayerEngine::handleRemoteInput(const Message& msg) {
    Message msgCopy = msg;
    uint8_t actionByte;
    uint32_t frameNum;
    
    if (!(msgCopy.payload() >> actionByte >> frameNum)) {
        return;
    }
    
    InputAction action = static_cast<InputAction>(actionByte);
    
    if (m_networkSession->getRole() == NetworkSession::SessionRole::HOST) {
        if (!validateInput(action, m_remoteGameState)) {
            return;
        }
    }
    
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
