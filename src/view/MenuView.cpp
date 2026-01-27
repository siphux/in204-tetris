#include "MenuView.h"
#include <string>

// MenuView: Handles rendering of all menus (main menu, mode selection, etc.)

// Initialize the menu view - try to load a font for text
MenuView::MenuView() : m_fontLoaded(false) {
    // Try to load a system font (different paths for different operating systems)
    if (!m_font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") &&  // Linux
        !m_font.openFromFile("/System/Library/Fonts/Arial.ttf") &&  // macOS
        !m_font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {  // Windows
        m_fontLoaded = false;  // Couldn't load any font
    } else {
        m_fontLoaded = true;  // Successfully loaded a font
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
    drawMenuOption(window, "Multiplayer", 250.0f + OPTION_SPACING, selectedOption == 1);
    drawMenuOption(window, "Exit", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
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
    drawMenuOption(window, "AI Mode", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
    drawMenuOption(window, "Back", 250.0f + 3 * OPTION_SPACING, selectedOption == 3);
}

void MenuView::renderAISelection(sf::RenderWindow& window, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "Select AI Type", 100.0f, TITLE_SIZE, sf::Color::White);

    // Draw AI options
    drawMenuOption(window, "Simple AI", 250.0f, selectedOption == 0);
    drawMenuOption(window, "Advanced AI (with lookahead)", 250.0f + OPTION_SPACING, selectedOption == 1);
    drawMenuOption(window, "Back", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
}

void MenuView::renderMultiplayerMenu(sf::RenderWindow& window, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "Multiplayer", 100.0f, TITLE_SIZE, sf::Color::White);

    // Draw options
    drawMenuOption(window, "Local Multiplayer", 250.0f, selectedOption == 0);
    drawMenuOption(window, "LAN Multiplayer", 250.0f + OPTION_SPACING, selectedOption == 1);
    drawMenuOption(window, "Back", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
}

void MenuView::renderLocalMultiplayerMenu(sf::RenderWindow& window, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "Local Multiplayer", 100.0f, TITLE_SIZE, sf::Color::White);

    // Draw options
    drawMenuOption(window, "AI vs AI", 250.0f, selectedOption == 0);
    drawMenuOption(window, "Player vs AI", 250.0f + OPTION_SPACING, selectedOption == 1);
    drawMenuOption(window, "Back", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
}

void MenuView::renderLANMultiplayerMenu(sf::RenderWindow& window, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "LAN Multiplayer", 100.0f, TITLE_SIZE, sf::Color::White);

    // Draw options
    drawMenuOption(window, "Host Game", 250.0f, selectedOption == 0);
    drawMenuOption(window, "Join Game", 250.0f + OPTION_SPACING, selectedOption == 1);
    drawMenuOption(window, "Back", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
}

void MenuView::renderHostGame(sf::RenderWindow& window, const std::string& localIP, bool connected) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "Hosting Game", 100.0f, TITLE_SIZE, sf::Color::White);

    // Show local IP
    drawCenteredText(window, "Your IP: " + localIP, 200.0f, OPTION_SIZE, sf::Color::Cyan);

    // Show status
    if (connected) {
        drawCenteredText(window, "Player Connected!", 280.0f, OPTION_SIZE, sf::Color::Green);
        drawCenteredText(window, "Starting game...", 340.0f, OPTION_SIZE - 6, sf::Color::White);
    } else {
        drawCenteredText(window, "Waiting for connection...", 280.0f, OPTION_SIZE, sf::Color::Yellow);
        drawCenteredText(window, "Press ESC to cancel", 400.0f, OPTION_SIZE - 8, sf::Color(150, 150, 150));
    }
}

void MenuView::renderJoinGame(sf::RenderWindow& window, int selectedOption, const std::string& ipInput) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "Join Game", 100.0f, TITLE_SIZE, sf::Color::White);

    // Draw instruction
    drawCenteredText(window, "Enter host IP address:", 200.0f, OPTION_SIZE - 4, sf::Color::White);

    // Draw IP input box
    float boxWidth = 400.0f;
    float boxHeight = 50.0f;
    float boxX = (window.getSize().x - boxWidth) / 2;
    float boxY = 260.0f;

    sf::RectangleShape inputBox({boxWidth, boxHeight});
    inputBox.setPosition({boxX, boxY});
    inputBox.setFillColor(sf::Color(40, 40, 40));
    inputBox.setOutlineThickness(2.0f);
    inputBox.setOutlineColor(sf::Color::Cyan);
    window.draw(inputBox);

    // Draw IP text
    if (m_fontLoaded) {
        sf::Text ipText(m_font, ipInput.empty() ? "192.168.x.x" : ipInput, 32);
        ipText.setFillColor(ipInput.empty() ? sf::Color(100, 100, 100) : sf::Color::White);
        sf::FloatRect bounds = ipText.getLocalBounds();
        ipText.setPosition(sf::Vector2f(boxX + 10, boxY + 10));
        window.draw(ipText);
    }

    // Draw options
    drawMenuOption(window, "Connect", 360.0f, selectedOption == 0);
    drawMenuOption(window, "Back", 360.0f + OPTION_SPACING, selectedOption == 1);
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
    float optionY = boxY + 120.0f;
    drawMenuOption(window, "Resume", optionY, selectedOption == 0);
    drawMenuOption(window, "Main Menu", optionY + OPTION_SPACING, selectedOption == 1);
}

void MenuView::renderGameOver(sf::RenderWindow& window, int player1Score, int player1Lines, int selectedOption,
                             bool isMultiplayer, int winnerId, const std::string& winnerName,
                             int player2Score, int player2Lines) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title based on multiplayer or singleplayer
    if (isMultiplayer && winnerId != -1) {
        // Multiplayer: Show winner
        drawCenteredText(window, "GAME OVER", 80.0f, TITLE_SIZE, sf::Color::Red);
        
        std::string winnerText = winnerName.empty() ? ("Player " + std::to_string(winnerId + 1) + " Wins!") 
                                                     : (winnerName + " Wins!");
        drawCenteredText(window, winnerText, 160.0f, OPTION_SIZE + 8, sf::Color::Yellow);
        
        // Draw both players' stats
        drawCenteredText(window, "Player 1 - Score: " + std::to_string(player1Score) + ", Lines: " + std::to_string(player1Lines),
                        230.0f, OPTION_SIZE - 4, sf::Color::White);
        drawCenteredText(window, "Player 2 - Score: " + std::to_string(player2Score) + ", Lines: " + std::to_string(player2Lines),
                        280.0f, OPTION_SIZE - 4, sf::Color::White);
    } else {
        // Singleplayer: Normal game over
        drawCenteredText(window, "GAME OVER", 100.0f, TITLE_SIZE, sf::Color::Red);

        // Draw score
        drawCenteredText(window, "Final Score: " + std::to_string(player1Score), 
                        200.0f, OPTION_SIZE, sf::Color::White);
        
        // Draw lines cleared
        drawCenteredText(window, "Lines Cleared: " + std::to_string(player1Lines), 
                        260.0f, OPTION_SIZE, sf::Color::White);
    }

    // Draw options
    drawMenuOption(window, "Play Again", 350.0f, selectedOption == 0);
    drawMenuOption(window, "Main Menu", 350.0f + OPTION_SPACING, selectedOption == 1);
}

int MenuView::getOptionCount(MenuState menuState) const {
    switch (menuState) {
        case MenuState::MAIN_MENU:
            return 3; // Start Game, Multiplayer, Exit
        case MenuState::MODE_SELECTION:
            return 4; // Level Mode, Deathrun Mode, AI Mode, Back
        case MenuState::AI_SELECTION:
            return 3; // Simple AI, Advanced AI, Back
        case MenuState::MULTIPLAYER_MENU:
            return 3; // Local Multiplayer, LAN Multiplayer, Back
        case MenuState::LOCAL_MULTIPLAYER:
            return 3; // AI vs AI, Player vs AI, Back
        case MenuState::LAN_MULTIPLAYER:
            return 3; // Host Game, Join Game, Back
        case MenuState::HOST_GAME:
            return 0; // No options, waiting for connection
        case MenuState::JOIN_GAME:
            return 2; // Connect, Back
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
