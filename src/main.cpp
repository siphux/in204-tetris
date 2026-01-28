#include <SFML/Graphics.hpp>
#include "controller/GameController.h"
#include "view/GameView.h"
#include "ConfigManager.h"
#include <cstdlib>
#include <ctime>

int main() {
    //Loading the configuration of the game stored in the config.ini file
    ConfigManager& config = ConfigManager::getInstance();
    config.load("config.ini");
    // Initialize a random seed    
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Get window configuration from config
    int windowWidth = config.getWindowWidth();
    int windowHeight = config.getWindowHeight();
    int fpsLimit = config.getFPSLimit();

    // Create main window using SFML
    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned int>(windowWidth), 
                                          static_cast<unsigned int>(windowHeight)}), "Tetris");
    
    
    
    //Setting up the window position
    auto desktopMode = sf::VideoMode::getDesktopMode();
    
    int centerX = (desktopMode.size.x - windowWidth) / 2;
    int centerY = (desktopMode.size.y - windowHeight) / 2;
    
    if (centerX > 2000) {
        centerX = (1920 - windowWidth) / 2;
        centerY = (1080 - windowHeight) / 2;
    }
    
    if (centerX < 50) centerX = 50;
    if (centerY < 50) centerY = 50;
    
    window.setPosition({centerX, centerY});
    window.setFramerateLimit(fpsLimit);
    
    // Disable key repeat to avoid multiple inputs on single press
    window.setKeyRepeatEnabled(false);

    //preparing main loop
    GameController controller;
    GameView view;

    sf::Clock clock;
    // Main loop
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        while (const auto event = window.pollEvent()) {
            
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            controller.handleEvent(*event);
        }

        controller.update(deltaTime);
        
        // Check if user requested exit
        if (controller.shouldExit()) {
            window.close();
        }
        // Render current game state and menu taking into account multiplayer status
        const bool isMultiplayer = controller.isLocalMultiplayerMode() || controller.isNetworkMultiplayerMode();
        const GameState* remoteState = isMultiplayer ? &controller.getRemoteGameState() : nullptr;
        
        // Get connection status and IP for network menus
        const bool isNetworkConnected = controller.isNetworkConnected();
        const std::string localIP = controller.getLocalIP();
        
        view.render(window, controller.getGameState(), controller.getMenuView(),
                   controller.getMenuState(), controller.getSelectedOption(),
                   isMultiplayer, remoteState,
                   controller.getWinnerId(), controller.getWinnerName(),
                   isNetworkConnected, localIP,
                   controller.getLocalPlayerReady(), controller.getRemotePlayerReady(),
                   controller.getMusicVolume());
        
        view.setIPInput(controller.getIPInput());
    }

    return 0;
}