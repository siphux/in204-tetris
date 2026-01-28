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
    LOCAL_MULTIPLAYER,
    LAN_MULTIPLAYER,
    HOST_GAME,
    JOIN_GAME,
    NETWORK_READY,  // ready screen that both players need to confirm before starting LAN multiplayer game
    PAUSE_MENU,
    SETTINGS_MENU,
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

    // Render local multiplayer menu
    void renderLocalMultiplayerMenu(sf::RenderWindow& window, int selectedOption) const;

    // Render LAN multiplayer menu
    void renderLANMultiplayerMenu(sf::RenderWindow& window, int selectedOption) const;

    // Render host game menu (waiting for connection)
    void renderHostGame(sf::RenderWindow& window, const std::string& localIP, bool connected) const;

    // Render join game menu (enter IP)
    void renderJoinGame(sf::RenderWindow& window, int selectedOption, const std::string& ipInput) const;

    // Render network ready menu (waiting for both players to confirm ready)
    void renderNetworkReady(sf::RenderWindow& window, int selectedOption, bool localReady, bool remoteReady) const;

    // Render pause menu
    void renderPauseMenu(sf::RenderWindow& window, int selectedOption, float musicVolume) const;

    // Render settings menu
    void renderSettingsMenu(sf::RenderWindow& window, int selectedOption, float musicVolume) const;

    // Render game over screen with final score and lines cleared
    void renderGameOver(sf::RenderWindow& window, int player1Score, int player1Lines, int selectedOption,
                       bool isMultiplayer = false, int winnerId = -1, const std::string& winnerName = "",
                       int player2Score = 0, int player2Lines = 0) const;

    // Get number of options in current menu
    int getOptionCount(MenuState menuState) const;

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
