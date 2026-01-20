#include <SFML/Graphics.hpp>
#include <optional>

int main() {
    /*
    For the time being, a test function to see if SFML is working.
    */
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Tetris - IN204-TETRIS");
    window.setFramerateLimit(60);
    
    // Center window on primary monitor
    // Get desktop video mode to calculate center position
    auto desktopMode = sf::VideoMode::getDesktopMode();
    int centerX = (desktopMode.size.x - 800) / 2;
    int centerY = (desktopMode.size.y - 600) / 2;
    window.setPosition({centerX, centerY});
    
    // Test: Draw a simple rectangle
    sf::RectangleShape testBlock(sf::Vector2f(50.f, 50.f));
    testBlock.setFillColor(sf::Color::Cyan);
    testBlock.setPosition({375.f, 275.f});
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }
        }
        
        window.clear(sf::Color::Black);
        window.draw(testBlock);
        window.display();
    }
    
    return 0;
}