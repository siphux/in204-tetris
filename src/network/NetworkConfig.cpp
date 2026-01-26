#include "NetworkConfig.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Get the single instance of NetworkConfig (singleton pattern)
NetworkConfig& NetworkConfig::getInstance() {
    static NetworkConfig instance;
    return instance;
}

NetworkConfig::NetworkConfig()
    : m_port(53000),
      m_connectionTimeoutMs(15000),
      m_reconnectDelayMs(3000),
      m_maxReconnectAttempts(5),
      m_heartbeatIntervalSeconds(2),
      m_connectionTimeoutSeconds(10),
      m_defaultTargetLines(40),
      m_enableScreenShareRace(false),
      m_aiMoveDelay(0.2f) {
    loadFromFile();
}

// Load settings from config.ini file
void NetworkConfig::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // File doesn't exist - use default values
        return;
    }
    
    std::string line;
    std::string currentSection = "";  // Which section we're in: "Network", "Game", etc.
    
    // Read file line by line
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        // Check if this is a section header like [Network]
        if (line[0] == '[' && line.back() == ']') {
            // Extract section name (remove [ and ])
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Find the = sign that separates key from value
        size_t equalsPos = line.find('=');
        if (equalsPos == std::string::npos) {
            continue;  // No = sign, skip this line
        }
        
        // Split line into key and value
        std::string key = line.substr(0, equalsPos);
        std::string value = line.substr(equalsPos + 1);
        
        // Remove spaces from start and end of key and value
        while (!key.empty() && (key[0] == ' ' || key[0] == '\t')) {
            key.erase(0, 1);
        }
        while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) {
            key.pop_back();
        }
        while (!value.empty() && (value[0] == ' ' || value[0] == '\t')) {
            value.erase(0, 1);
        }
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t')) {
            value.pop_back();
        }
        
        // Read Network section settings
        if (currentSection == "Network") {
            if (key == "port") {
                m_port = static_cast<unsigned short>(std::stoi(value));
            } else if (key == "connection_timeout_ms") {
                m_connectionTimeoutMs = std::stoi(value);
            } else if (key == "reconnect_delay_ms") {
                m_reconnectDelayMs = std::stoi(value);
            } else if (key == "max_reconnect_attempts") {
                m_maxReconnectAttempts = std::stoi(value);
            } else if (key == "heartbeat_interval_seconds") {
                m_heartbeatIntervalSeconds = std::stoi(value);
            } else if (key == "connection_timeout_seconds") {
                m_connectionTimeoutSeconds = std::stoi(value);
            }
        }
        // Read Game section settings
        else if (currentSection == "Game") {
            if (key == "default_target_lines") {
                m_defaultTargetLines = std::stoi(value);
            } else if (key == "enable_screen_share_race") {
                m_enableScreenShareRace = (value == "true" || value == "1");
            } else if (key == "ai_move_delay") {
                m_aiMoveDelay = std::stof(value);
            }
        }
    }
}

void NetworkConfig::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }
    
    file << "[Network]\n";
    file << "port=" << m_port << "\n";
    file << "connection_timeout_ms=" << m_connectionTimeoutMs << "\n";
    file << "reconnect_delay_ms=" << m_reconnectDelayMs << "\n";
    file << "max_reconnect_attempts=" << m_maxReconnectAttempts << "\n";
    file << "heartbeat_interval_seconds=" << m_heartbeatIntervalSeconds << "\n";
    file << "connection_timeout_seconds=" << m_connectionTimeoutSeconds << "\n";
    file << "\n";
    file << "[Game]\n";
    file << "default_target_lines=" << m_defaultTargetLines << "\n";
    file << "enable_screen_share_race=" << (m_enableScreenShareRace ? "true" : "false") << "\n";
    file << "ai_move_delay=" << m_aiMoveDelay << "\n";
    file << "\n";
    file << "[Display]\n";
    file << "window_width=800\n";
    file << "window_height=700\n";
    file << "fps_limit=60\n";
}
