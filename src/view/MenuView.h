#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// Enum for menu types
enum class MenuState {
    MAIN_MENU,
    MODE_SELECTION,
    AI_SELECTION,
    MULTIPLAYER_MENU,
    HOST_GAME,
    JOIN_GAME,
    ENTER_IP,
    PAUSE_MENU,
    GAME_OVER,
    NONE  // No menu active
};

// Handles rendering menus and menu selection UI
class MenuView {
public:
    MenuView();

    // Render main menu with game mode selection
    void renderMainMenu(sf::RenderWindow& window, int selectedOption) const;

    // Render mode selection menu
    void renderModeSelection(sf::RenderWindow& window, int selectedOption) const;

    // Render AI selection menu
    void renderAISelection(sf::RenderWindow& window, int selectedOption) const;

    // Render multiplayer menu
    void renderMultiplayerMenu(sf::RenderWindow& window, int selectedOption) const;

    // Render host game menu (waiting for connection)
    void renderHostGame(sf::RenderWindow& window, bool isConnected, 
                       const std::string& localIP = "", const std::string& publicIP = "", int selectedOption = 0) const;

    // Render join game menu
    void renderJoinGame(sf::RenderWindow& window, int selectedOption) const;

    // Render IP input menu
    void renderEnterIP(sf::RenderWindow& window, const std::string& currentIP) const;

    // Render pause menu
    void renderPauseMenu(sf::RenderWindow& window, int selectedOption, bool isMultiplayer = false) const;

    // Render game over screen with final score
    void renderGameOver(sf::RenderWindow& window, int finalScore, int selectedOption) const;

    // Get number of options in current menu
    int getOptionCount(MenuState menuState, bool isMultiplayer = false) const;

private:
    sf::Font m_font;
    bool m_fontLoaded;

    static constexpr float TITLE_SIZE = 48.0f;
    static constexpr float OPTION_SIZE = 36.0f;
    static constexpr float OPTION_SPACING = 60.0f;

    void drawCenteredText(sf::RenderWindow& window, const std::string& text,
                         float y, float size, const sf::Color& color) const;

    void drawMenuOption(sf::RenderWindow& window, const std::string& text,
                       float y, bool isSelected) const;
};
