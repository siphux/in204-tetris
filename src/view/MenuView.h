#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// Enum for menu types
enum class MenuState {
    MAIN_MENU,
    MODE_SELECTION,
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

    // Render pause menu
    void renderPauseMenu(sf::RenderWindow& window, int selectedOption) const;

    // Render game over screen with final score
    void renderGameOver(sf::RenderWindow& window, int finalScore, int selectedOption) const;

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
