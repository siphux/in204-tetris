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
    drawMenuOption(window, "Host Game", 250.0f, selectedOption == 0);
    drawMenuOption(window, "Join Game", 250.0f + OPTION_SPACING, selectedOption == 1);
    drawMenuOption(window, "Back", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
}

void MenuView::renderHostGame(sf::RenderWindow& window, bool isConnected, 
                             const std::string& localIP, const std::string& publicIP, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "Hosting Game", 100.0f, TITLE_SIZE, sf::Color::White);

    if (isConnected) {
        drawCenteredText(window, "Player connected! Game starting...", 
                        250.0f, OPTION_SIZE, sf::Color::Green);
    } else {
        drawCenteredText(window, "Waiting for player to connect...", 
                        250.0f, OPTION_SIZE, sf::Color::Yellow);
        
        float yPos = 300.0f;
        
        // Display public IP for internet play (preferred)
        if (!publicIP.empty() && publicIP.find("public") == std::string::npos) {
            std::string ipText = "Internet: " + publicIP;
            drawCenteredText(window, ipText, 
                            yPos, OPTION_SIZE * 0.9f, sf::Color::Green);
            yPos += 40.0f;
            drawCenteredText(window, "Share this with your friend", 
                            yPos, OPTION_SIZE * 0.7f, sf::Color::White);
            yPos += 50.0f;
        }
        
        // Display local IP for LAN play (fallback)
        if (!localIP.empty() && localIP != "Unknown") {
            std::string ipText = "Local Network: " + localIP;
            drawCenteredText(window, ipText, 
                            yPos, OPTION_SIZE * 0.9f, sf::Color::Cyan);
            yPos += 40.0f;
        }
        
        if (publicIP.empty() || publicIP.find("public") != std::string::npos) {
            drawCenteredText(window, "Fetching public IP...", 
                            yPos, OPTION_SIZE * 0.7f, sf::Color::Yellow);
            yPos += 30.0f;
        }
        
        // Draw cancel option
        drawMenuOption(window, "Cancel", 450.0f, selectedOption == 0);
    }
}

void MenuView::renderJoinGame(sf::RenderWindow& window, int selectedOption) const {
    // Draw background
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    // Draw title
    drawCenteredText(window, "Join Game", 100.0f, TITLE_SIZE, sf::Color::White);

    // Draw connection options
    drawMenuOption(window, "Connect to localhost", 250.0f, selectedOption == 0);
    drawMenuOption(window, "Enter IP address", 250.0f + OPTION_SPACING, selectedOption == 1);
    drawMenuOption(window, "Back", 250.0f + 2 * OPTION_SPACING, selectedOption == 2);
}

void MenuView::renderEnterIP(sf::RenderWindow& window, const std::string& currentIP, int selectedOption) const {
    sf::RectangleShape background({static_cast<float>(window.getSize().x),
                                  static_cast<float>(window.getSize().y)});
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    drawCenteredText(window, "Enter Server Address", 100.0f, TITLE_SIZE, sf::Color::White);

    std::string ipDisplay = "Address: " + (currentIP.empty() ? "..." : currentIP);
    drawCenteredText(window, ipDisplay, 250.0f, OPTION_SIZE, 
                     selectedOption == 0 ? sf::Color::Yellow : sf::Color::White);
    
    drawCenteredText(window, "Format: IP:PORT (e.g., 192.168.1.100:53000)", 
                    300.0f, OPTION_SIZE * 0.7f, sf::Color::White);
    drawCenteredText(window, "Port is optional (default: 53000)", 
                    330.0f, OPTION_SIZE * 0.6f, sf::Color::Cyan);
    
    drawMenuOption(window, "Back", 400.0f, selectedOption == 1);
    
    drawCenteredText(window, "Backspace to delete | Enter to connect", 
                    460.0f, OPTION_SIZE * 0.6f, sf::Color::Cyan);
}

void MenuView::renderPauseMenu(sf::RenderWindow& window, int selectedOption, bool isMultiplayer) const {
    // Draw semi-transparent overlay
    sf::RectangleShape overlay({static_cast<float>(window.getSize().x),
                               static_cast<float>(window.getSize().y)});
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    // Draw pause menu box
    float boxWidth = 400.0f;
    float boxHeight = isMultiplayer ? 360.0f : 300.0f;  // Taller if multiplayer
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
    
    if (isMultiplayer) {
        // Multiplayer mode: Show disconnect option
        drawMenuOption(window, "Disconnect", optionY + OPTION_SPACING, selectedOption == 1);
        drawMenuOption(window, "Main Menu", optionY + 2 * OPTION_SPACING, selectedOption == 2);
    } else {
        // Solo mode: Just main menu
        drawMenuOption(window, "Main Menu", optionY + OPTION_SPACING, selectedOption == 1);
    }
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

int MenuView::getOptionCount(MenuState menuState, bool isMultiplayer) const {
    switch (menuState) {
        case MenuState::MAIN_MENU:
            return 3; // Start Game, Multiplayer, Exit
        case MenuState::MODE_SELECTION:
            return 4; // Level Mode, Deathrun Mode, AI Mode, Back
        case MenuState::AI_SELECTION:
            return 3; // Simple AI, Advanced AI, Back
        case MenuState::MULTIPLAYER_MENU:
            return 3; // Host Game, Join Game, Back
        case MenuState::HOST_GAME:
            return 1; // Cancel option
        case MenuState::JOIN_GAME:
            return 3; // Connect to localhost, Enter IP address, Back
        case MenuState::ENTER_IP:
            return 2; // Input field (option 0) and Back (option 1)
        case MenuState::PAUSE_MENU:
            return isMultiplayer ? 3 : 2; // Resume, Disconnect/Main Menu, Main Menu (if multiplayer)
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
