#include "InputHandler.h"

InputHandler::InputHandler()
    : m_leftPressed(false),
      m_rightPressed(false),
      m_downPressed(false),
      m_rotatePressed(false) {}

void InputHandler::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Left:
            m_leftPressed = true;
            break;
        case sf::Keyboard::Key::Right:
            m_rightPressed = true;
            break;
        case sf::Keyboard::Key::Down:
            m_downPressed = true;
            break;
        case sf::Keyboard::Key::Up:
        case sf::Keyboard::Key::Space:
            m_rotatePressed = true;
            break;
        default:
            break;
    }
}

void InputHandler::handleKeyRelease(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Left:
            m_leftPressed = false;
            break;
        case sf::Keyboard::Key::Right:
            m_rightPressed = false;
            break;
        case sf::Keyboard::Key::Down:
            m_downPressed = false;
            break;
        case sf::Keyboard::Key::Up:
        case sf::Keyboard::Key::Space:
            m_rotatePressed = false;
            break;
        default:
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

bool InputHandler::isRotatePressed() const {
    return m_rotatePressed;
}

void InputHandler::resetRotateFlag() {
    m_rotatePressed = false;
}
