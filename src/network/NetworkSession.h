#pragma once

#include <SFML/Network.hpp>
#include <string>
#include <memory>
#include <cstdint>
#include <chrono>

class NetworkSession {
public:
    enum class SessionRole {
        HOST,
        CLIENT
    };

    enum class ConnectionStatus {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        ERROR
    };

    NetworkSession(SessionRole role, const std::string& playerName);
    ~NetworkSession();

    bool startHosting(unsigned short port = 0);
    unsigned short getListeningPort() const;
    std::string getLocalIP() const;
    bool pollIncomingConnection();

    bool connectToHost(const std::string& hostAddress, unsigned short port, int timeoutMs = -1);

    bool sendPacket(const uint8_t* data, size_t size);
    size_t receivePacket(uint8_t* outData, size_t maxSize);
    
    ConnectionStatus getStatus() const { return m_status; }
    bool isConnected() const { return m_status == ConnectionStatus::CONNECTED; }
    
    std::string getRemotePlayerName() const { return m_remotePlayerName; }
    uint32_t getLatencyMs() const { return m_latencyMs; }
    
    void sendHeartbeat();
    void update();
    void disconnect();
    void setBlocking(bool blocking);
    SessionRole getRole() const { return m_role; }
    std::string getLastError() const { return m_lastError; }
    
    bool attemptReconnect(const std::string& hostAddress, unsigned short port, int timeoutMs = 15000);
    void setReconnectInfo(const std::string& hostAddress, unsigned short port);
    bool shouldAttemptReconnect() const;

private:
    SessionRole m_role;
    ConnectionStatus m_status;
    std::string m_playerName;
    std::string m_remotePlayerName;
    std::string m_lastError;

    std::unique_ptr<sf::TcpListener> m_listener;
    std::unique_ptr<sf::TcpSocket> m_socket;

    uint32_t m_latencyMs;
    std::chrono::steady_clock::time_point m_lastHeartbeatSent;
    std::chrono::steady_clock::time_point m_lastHeartbeatReceived;
    std::chrono::steady_clock::time_point m_connectionStartTime;
    
    std::string m_reconnectHost;
    unsigned short m_reconnectPort;
    std::chrono::steady_clock::time_point m_lastReconnectAttempt;
    static constexpr int RECONNECT_DELAY_MS = 3000;
    int m_reconnectAttempts;
    static constexpr int MAX_RECONNECT_ATTEMPTS = 5;

    void updateConnectionTimeout();
    void updateHeartbeat();
    std::string getSystemLocalIP() const;
};
