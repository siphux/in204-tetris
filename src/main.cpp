#include <SFML/Graphics.hpp>
#include "controller/GameController.h"
#include "view/GameView.h"
#include "network/NetworkConfig.h"
#include <cstdlib>
#include <ctime>

int main() {
    // Load network and game settings from config.ini
    NetworkConfig::getInstance().loadFromFile();
    
    // Seed the random number generator so we get different piece sequences each game
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Create window first
    sf::RenderWindow window(sf::VideoMode({800, 700}), "Tetros");
    
    // Get all available video modes to find primary monitor
    auto desktopMode = sf::VideoMode::getDesktopMode();
    
    // For multi-monitor setups, try to center on primary monitor
    // If getDesktopMode() returns combined size, use a reasonable default
    // Most common primary monitor sizes: 1920x1080, 2560x1440, etc.
    int centerX = (desktopMode.size.x - 800) / 2;
    int centerY = (desktopMode.size.y - 700) / 2;
    
    // Clamp to reasonable values to ensure it's on primary monitor
    // If calculated position seems wrong (too far right), use a fixed offset
    if (centerX > 2000) {
        // Likely multi-monitor combined size, use fixed position
        centerX = (1920 - 800) / 2;  // Assume 1920x1080 primary
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

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }

            controller.handleEvent(*event);
        }

        controller.update(deltaTime);
        
        // Check if controller wants to exit
        if (controller.shouldExit()) {
            window.close();
        }
        
        const GameState* remoteState = controller.isNetworkMode() ? &controller.getRemoteGameState() : nullptr;
        view.render(window, controller.getGameState(), controller.getMenuView(),
                   controller.getMenuState(), controller.getSelectedOption(),
                   controller.isHosting(), controller.isClientConnected(),
                   controller.getIPInput(), controller.getServerLocalIP(),
                   controller.getServerPublicIP(), controller.isNetworkMode(),
                   controller.getLastConnectionError(),
                   remoteState, controller.getNetworkLatency());
    }

    return 0;
}