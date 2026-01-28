#pragma once
#include <string>
#include <map>
#include <iostream>

// Manages reading and accessing configuration from config.ini
class ConfigManager {
public:
    static ConfigManager& getInstance();
    
    // Load configuration from file
    bool load(const std::string& filename);
    
    // Network settings
    unsigned short getNetworkPort() const { return m_networkPort; }
    
    // Game settings
    int getDefaultTargetLines() const { return m_defaultTargetLines; }
    float getAIMoveDelay() const { return m_aiMoveDelay; }
    
    // Display settings
    int getWindowHeight() const { return m_windowHeight; }
    int getWindowWidth() const { return m_windowWidth; }
    int getFPSLimit() const { return m_fpsLimit; }
    
private:
    ConfigManager();
    
    // Parse INI-style configuration
    void parseConfigFile(const std::string& content);
    std::string trim(const std::string& str);
    
    // Default values
    unsigned short m_networkPort = 53000;
    int m_defaultTargetLines = 40;
    float m_aiMoveDelay = 0.2f;
    int m_windowHeight = 700;
    int m_windowWidth = 1400;
    int m_fpsLimit = 60;
};
