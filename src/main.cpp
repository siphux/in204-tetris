#include <SFML/Graphics.hpp>
#include "controller/GameController.h"
#include "view/GameView.h"
#include <cstdlib>
#include <ctime>

int main() {
    // Seed the random number generator so we get different piece sequences each game
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Create window first
    // Use wider window for better multiplayer view (1400x700)
    sf::RenderWindow window(sf::VideoMode({1400, 700}), "Tetros");
    
    // Get all available video modes to find primary monitor
    auto desktopMode = sf::VideoMode::getDesktopMode();
    
    // For multi-monitor setups, try to center on primary monitor
    // If getDesktopMode() returns combined size, use a reasonable default
    // Most common primary monitor sizes: 1920x1080, 2560x1440, etc.
    int centerX = (desktopMode.size.x - 1400) / 2;
    int centerY = (desktopMode.size.y - 700) / 2;
    
    // Clamp to reasonable values to ensure it's on primary monitor
    // If calculated position seems wrong (too far right), use a fixed offset
    if (centerX > 2000) {
        // Likely multi-monitor combined size, use fixed position
        centerX = (1920 - 1400) / 2;  // Assume 1920x1080 primary
        centerY = (1080 - 700) / 2;
    }
    
    // Ensure minimum offset from edges
    if (centerX < 50) centerX = 50;
    if (centerY < 50) centerY = 50;
    
    window.setPosition({centerX, centerY});
    window.setFramerateLimit(60);
    
    // Disable key repeat to prevent multiple key press events when holding keys
    // This makes input handling more predictable, especially in multiplayer
    window.setKeyRepeatEnabled(false);

    GameController controller;
    GameView view;

    sf::Clock clock;

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        while (const auto event = window.pollEvent()) {
            
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            controller.handleEvent(*event);
        }

        controller.update(deltaTime);
        
        // Check if controller wants to exit
        if (controller.shouldExit()) {
            window.close();
        }
        
        const bool isMultiplayer = controller.isLocalMultiplayerMode() || controller.isNetworkMultiplayerMode();
        const GameState* remoteState = isMultiplayer ? &controller.getRemoteGameState() : nullptr;
        
        // Get connection status and IP for network menus
        const bool isNetworkConnected = controller.isNetworkConnected();
        const std::string localIP = controller.getLocalIP();
        
        view.render(window, controller.getGameState(), controller.getMenuView(),
                   controller.getMenuState(), controller.getSelectedOption(),
                   isMultiplayer, remoteState,
                   controller.getWinnerId(), controller.getWinnerName(),
                   isNetworkConnected, localIP);
        
        // Update GameView's IP input for rendering
        view.setIPInput(controller.getIPInput());
    }

    return 0;
}