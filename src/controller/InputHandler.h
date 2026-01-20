#pragma once
#include <SFML/Window.hpp>

// Handles keyboard input for the game
// Uses SFML 3.0.2 API
class InputHandler {
public:
    InputHandler();
    
    // Process a key press event (for discrete actions like rotation)
    void handleKeyPress(sf::Keyboard::Key key);
    
    // Process a key release event
    void handleKeyRelease(sf::Keyboard::Key key);
    
    // Check if a key is currently pressed (for continuous actions like movement)
    bool isKeyPressed(sf::Keyboard::Key key) const;
    
    // Check movement keys
    bool isLeftPressed() const;
    bool isRightPressed() const;
    bool isDownPressed() const;
    bool isRotateClockwisePressed() const;
    bool isRotateCounterClockwisePressed() const;
    
    // Reset rotation flags (called after processing rotation)
    void resetRotateFlags();

private:
    bool m_leftPressed;
    bool m_rightPressed;
    bool m_downPressed;
    bool m_rotateClockwisePressed;
    bool m_rotateCounterClockwisePressed;
};
