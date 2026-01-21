#include "MenuView.h"
#include <string>

MenuView::MenuView() : m_fontLoaded(false) {
    // Try to load a system font
    if (!m_font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") &&
        !m_font.openFromFile("/System/Library/Fonts/Arial.ttf") &&
        !m_font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        m_fontLoaded = false;
    } else {
        m_fontLoaded = true;
    }
}

void MenuView::renderMainMenu(sf::RenderWindow& window, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "TETRIS", 100.0f, TITLE_SIZE, sf::Color::White);

    // Draw options
    drawMenuOption(window, "Start Game", 250.0f, selectedOption == 0);
    drawMenuOption(window, "Exit", 250.0f + OPTION_SPACING, selectedOption == 1);
}

void MenuView::renderModeSelection(sf::RenderWindow& window, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "Select Game Mode", 100.0f, TITLE_SIZE, sf::Color::White);

    // Draw mode options
    drawMenuOption(window, "Level Mode", 250.0f, selectedOption == 0);
    drawMenuOption(window, "Deathrun Mode", 250.0f + OPTION_SPACING, selectedOption == 1);
    drawMenuOption(window, "Back", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
}

void MenuView::renderPauseMenu(sf::RenderWindow& window, int selectedOption) const {
    // Draw semi-transparent overlay
    sf::RectangleShape overlay({static_cast<float>(window.getSize().x),
                               static_cast<float>(window.getSize().y)});
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    // Draw pause menu box
    float boxWidth = 400.0f;
    float boxHeight = 300.0f;
    float boxX = (window.getSize().x - boxWidth) / 2;
    float boxY = (window.getSize().y - boxHeight) / 2;

    sf::RectangleShape box({boxWidth, boxHeight});
    box.setPosition({boxX, boxY});
    box.setFillColor(sf::Color(50, 50, 50));
    box.setOutlineThickness(2.0f);
    box.setOutlineColor(sf::Color::White);
    window.draw(box);

    // Draw title
    drawCenteredText(window, "PAUSED", boxY + 30.0f, TITLE_SIZE * 0.75f, sf::Color::White);

    // Draw options
    drawMenuOption(window, "Resume", boxY + 120.0f, selectedOption == 0);
    drawMenuOption(window, "Main Menu", boxY + 120.0f + OPTION_SPACING, selectedOption == 1);
}

void MenuView::renderGameOver(sf::RenderWindow& window, int finalScore, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "GAME OVER", 100.0f, TITLE_SIZE, sf::Color::Red);

    // Draw score
    drawCenteredText(window, "Final Score: " + std::to_string(finalScore), 
                    200.0f, OPTION_SIZE, sf::Color::White);

    // Draw options
    drawMenuOption(window, "Play Again", 350.0f, selectedOption == 0);
    drawMenuOption(window, "Main Menu", 350.0f + OPTION_SPACING, selectedOption == 1);
}

int MenuView::getOptionCount(MenuState menuState) const {
    switch (menuState) {
        case MenuState::MAIN_MENU:
            return 2; // Start, Exit
        case MenuState::MODE_SELECTION:
            return 3; // Level Mode, Deathrun Mode, Back
        case MenuState::PAUSE_MENU:
            return 2; // Resume, Main Menu
        case MenuState::GAME_OVER:
            return 2; // Play Again, Main Menu
        default:
            return 0;
    }
}

void MenuView::drawCenteredText(sf::RenderWindow& window, const std::string& text,
                               float y, float size, const sf::Color& color) const {
    if (!m_fontLoaded) return;

    sf::Text sfText(m_font, text, static_cast<unsigned int>(size));
    sfText.setFillColor(color);
    sf::FloatRect bounds = sfText.getLocalBounds();
    sfText.setPosition(
        sf::Vector2f((window.getSize().x - bounds.size.x) / 2.0f, y)
    );
    window.draw(sfText);
}

void MenuView::drawMenuOption(sf::RenderWindow& window, const std::string& text,
                             float y, bool isSelected) const {
    if (!m_fontLoaded) return;

    sf::Text sfText(m_font, text, static_cast<unsigned int>(OPTION_SIZE));
    sf::Color textColor = isSelected ? sf::Color::Yellow : sf::Color::White;
    sfText.setFillColor(textColor);

    sf::FloatRect bounds = sfText.getLocalBounds();
    sfText.setPosition(
        sf::Vector2f((window.getSize().x - bounds.size.x) / 2.0f, y)
    );

    if (isSelected) {
        // Draw selection indicator
        sf::Text indicator(m_font, "> ", static_cast<unsigned int>(OPTION_SIZE));
        indicator.setFillColor(sf::Color::Yellow);
        indicator.setPosition(
            sf::Vector2f(sfText.getPosition().x - 40.0f, y)
        );
        window.draw(indicator);
    }

    window.draw(sfText);
}
