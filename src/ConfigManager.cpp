#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>

ConfigManager::ConfigManager() = default;

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file: " << filename << std::endl;
        std::cerr << "Using default configuration values" << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    parseConfigFile(buffer.str());
    
    std::cout << "Loaded configuration from: " << filename << std::endl;
    return true;
}

std::string ConfigManager::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

void ConfigManager::parseConfigFile(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    std::string currentSection;

    while (std::getline(iss, line)) {
        // Trim whitespace
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Check for section headers [Section]
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        // Parse key=value pairs
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, eqPos));
        std::string value = trim(line.substr(eqPos + 1));

        // Parse values based on section and key
        if (currentSection == "Network") {
            if (key == "port") {
                try {
                    m_networkPort = static_cast<unsigned short>(std::stoi(value));
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing network port: " << e.what() << std::endl;
                }
            }
        } else if (currentSection == "Game") {
            if (key == "default_target_lines") {
                try {
                    m_defaultTargetLines = std::stoi(value);
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing default_target_lines: " << e.what() << std::endl;
                }
            } else if (key == "ai_move_delay") {
                try {
                    m_aiMoveDelay = std::stof(value);
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing ai_move_delay: " << e.what() << std::endl;
                }
            }
        } else if (currentSection == "Display") {
            if (key == "window_height") {
                try {
                    m_windowHeight = std::stoi(value);
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing window_height: " << e.what() << std::endl;
                }
            } else if (key == "window_width") {
                try {
                    m_windowWidth = std::stoi(value);
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing window_width: " << e.what() << std::endl;
                }
            } else if (key == "fps_limit") {
                try {
                    m_fpsLimit = std::stoi(value);
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing fps_limit: " << e.what() << std::endl;
                }
            }
        }
    }
}
