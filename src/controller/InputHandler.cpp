#include "InputHandler.h"

// InputHandler: Tracks which keys are currently pressed
// This allows the game to handle continuous movement (holding down arrow keys)

// Initialize all keys as not pressed
InputHandler::InputHandler()
    : m_leftPressed(false),
      m_rightPressed(false),
      m_downPressed(false),
      m_rotateClockwisePressed(false),
      m_rotateCounterClockwisePressed(false),
      m_hardDropPressed(false),
      m_leftJustPressed(false),
      m_rightJustPressed(false),
      m_downJustPressed(false) {}

// Called when a key is pressed - remember that it's pressed
// Note: Key repeat is disabled in main.cpp, so this is only called on actual key presses
void InputHandler::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Left:
            m_leftPressed = true;  // Remember left arrow is pressed
            m_leftJustPressed = true;  // Mark as just pressed for immediate processing
            break;
        case sf::Keyboard::Key::Right:
            m_rightPressed = true;  // Remember right arrow is pressed
            m_rightJustPressed = true;  // Mark as just pressed for immediate processing
            break;
        case sf::Keyboard::Key::Down:
            m_downPressed = true;  // Remember down arrow is pressed (soft drop)
            m_downJustPressed = true;  // Mark as just pressed for immediate processing
            break;
        case sf::Keyboard::Key::Up:
            m_rotateClockwisePressed = true;  // Remember up arrow is pressed (rotate clockwise)
            break;
        case sf::Keyboard::Key::Z:
            m_rotateCounterClockwisePressed = true;  // Remember Z is pressed (rotate counter-clockwise)
            break;
        case sf::Keyboard::Key::Space:
            m_hardDropPressed = true;  // Remember space is pressed (hard drop)
            break;
        default:
            // Other keys - ignore them
            break;
    }
}

// Called when a key is released - remember that it's no longer pressed
void InputHandler::handleKeyRelease(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Left:
            m_leftPressed = false;  // Left arrow is no longer pressed
            break;
        case sf::Keyboard::Key::Right:
            m_rightPressed = false;  // Right arrow is no longer pressed
            break;
        case sf::Keyboard::Key::Down:
            m_downPressed = false;  // Down arrow is no longer pressed
            break;
        case sf::Keyboard::Key::Up:
            m_rotateClockwisePressed = false;  // Up arrow is no longer pressed
            break;
        case sf::Keyboard::Key::Z:
            m_rotateCounterClockwisePressed = false;  // Z is no longer pressed
            break;
        case sf::Keyboard::Key::Space:
            m_hardDropPressed = false;  // Space is no longer pressed
            break;
        default:
            // Other keys - ignore them
            break;
    }
}

bool InputHandler::isKeyPressed(sf::Keyboard::Key key) const {
    return sf::Keyboard::isKeyPressed(key);
}

bool InputHandler::isLeftPressed() const {
    return m_leftPressed;
}

bool InputHandler::isRightPressed() const {
    return m_rightPressed;
}

bool InputHandler::isDownPressed() const {
    return m_downPressed;
}

bool InputHandler::isRotateClockwisePressed() const {
    return m_rotateClockwisePressed;
}

bool InputHandler::isRotateCounterClockwisePressed() const {
    return m_rotateCounterClockwisePressed;
}

bool InputHandler::isHardDropPressed() const {
    return m_hardDropPressed;
}

void InputHandler::resetRotateFlags() {
    m_rotateClockwisePressed = false;
    m_rotateCounterClockwisePressed = false;
}

void InputHandler::resetHardDropFlag() {
    m_hardDropPressed = false;
}

bool InputHandler::wasKeyJustPressed(sf::Keyboard::Key key) const {
    switch (key) {
        case sf::Keyboard::Key::Left:
            return m_leftJustPressed;
        case sf::Keyboard::Key::Right:
            return m_rightJustPressed;
        case sf::Keyboard::Key::Down:
            return m_downJustPressed;
        default:
            return false;
    }
}

void InputHandler::markKeyProcessed(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Left:
            m_leftJustPressed = false;
            break;
        case sf::Keyboard::Key::Right:
            m_rightJustPressed = false;
            break;
        case sf::Keyboard::Key::Down:
            m_downJustPressed = false;
            break;
        default:
            break;
    }
}
