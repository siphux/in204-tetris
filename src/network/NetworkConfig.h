#pragma once

#include <string>
#include <cstdint>

// NetworkConfig: Stores all network and game settings
// This is a "singleton" - there's only one instance in the whole program
// You can access it from anywhere using getInstance()
class NetworkConfig {
public:
    // Get the single instance of NetworkConfig
    static NetworkConfig& getInstance();
    
    // Load settings from config.ini file
    void loadFromFile(const std::string& filename = "config.ini");
    
    // Save current settings to config.ini file (not currently used)
    void saveToFile(const std::string& filename = "config.ini") const;
    
    unsigned short getPort() const { return m_port; }
    void setPort(unsigned short port) { m_port = port; }
    
    int getConnectionTimeoutMs() const { return m_connectionTimeoutMs; }
    void setConnectionTimeoutMs(int ms) { m_connectionTimeoutMs = ms; }
    
    int getReconnectDelayMs() const { return m_reconnectDelayMs; }
    void setReconnectDelayMs(int ms) { m_reconnectDelayMs = ms; }
    
    int getMaxReconnectAttempts() const { return m_maxReconnectAttempts; }
    void setMaxReconnectAttempts(int attempts) { m_maxReconnectAttempts = attempts; }
    
    int getHeartbeatIntervalSeconds() const { return m_heartbeatIntervalSeconds; }
    void setHeartbeatIntervalSeconds(int seconds) { m_heartbeatIntervalSeconds = seconds; }
    
    int getConnectionTimeoutSeconds() const { return m_connectionTimeoutSeconds; }
    void setConnectionTimeoutSeconds(int seconds) { m_connectionTimeoutSeconds = seconds; }
    
    int getDefaultTargetLines() const { return m_defaultTargetLines; }
    void setDefaultTargetLines(int lines) { m_defaultTargetLines = lines; }
    
    bool isScreenShareEnabledRace() const { return m_enableScreenShareRace; }
    void setScreenShareEnabledRace(bool enabled) { m_enableScreenShareRace = enabled; }
    
    float getAIMoveDelay() const { return m_aiMoveDelay; }
    void setAIMoveDelay(float delay) { m_aiMoveDelay = delay; }

private:
    NetworkConfig();
    ~NetworkConfig() = default;
    NetworkConfig(const NetworkConfig&) = delete;
    NetworkConfig& operator=(const NetworkConfig&) = delete;
    
    unsigned short m_port;
    int m_connectionTimeoutMs;
    int m_reconnectDelayMs;
    int m_maxReconnectAttempts;
    int m_heartbeatIntervalSeconds;
    int m_connectionTimeoutSeconds;
    int m_defaultTargetLines;
    bool m_enableScreenShareRace;
    float m_aiMoveDelay;
};
