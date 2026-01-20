#pragma once
#include "../model/GameState.h"
#include "InputHandler.h"
#include <SFML/Window/Event.hpp>

// GameController manages the game loop and coordinates between
// input handling, game state updates, and rendering
class GameController {
public:
    GameController();
    
    // Process SFML events (call this in your main loop)
    void handleEvent(const sf::Event& event);
    
    // Update game logic (call this every frame with deltaTime)
    void update(float deltaTime);
    
    // Get the current game state (for rendering)
    const GameState& getGameState() const;
    GameState& getGameState();
    
    // Check if game is over
    bool isGameOver() const;

private:
    GameState m_gameState;
    InputHandler m_inputHandler;
    
    // Input timing for movement (to prevent too-fast movement)
    float m_inputTimer;
    static constexpr float INPUT_DELAY = 0.1f; // 100ms delay between moves
    
    // Process continuous input (movement keys)
    void processContinuousInput();
    
    // Process discrete input (rotation)
    void processDiscreteInput();
};
