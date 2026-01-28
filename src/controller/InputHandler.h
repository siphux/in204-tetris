#pragma once
#include <SFML/Window.hpp>

//Handling user inputs for game control
class InputHandler {
public:
    InputHandler();
    void handleKeyPress(sf::Keyboard::Key key);
    void handleKeyRelease(sf::Keyboard::Key key);
    
    bool isKeyPressed(sf::Keyboard::Key key) const;
    
    bool isLeftPressed() const;
    bool isRightPressed() const;
    bool isDownPressed() const;
    bool isRotateClockwisePressed() const;
    bool isRotateCounterClockwisePressed() const;
    bool isHardDropPressed() const;
    void resetRotateFlags();
    
    void resetHardDropFlag();
    
    bool wasKeyJustPressed(sf::Keyboard::Key key) const;
    
    void markKeyProcessed(sf::Keyboard::Key key);

private:
    bool m_leftPressed;
    bool m_rightPressed;
    bool m_downPressed;
    bool m_rotateClockwisePressed;
    bool m_rotateCounterClockwisePressed;
    bool m_hardDropPressed;
    bool m_leftJustPressed;
    bool m_rightJustPressed;
    bool m_downJustPressed;
};
