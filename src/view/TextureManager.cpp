#include "TextureManager.h"
#include <cmath>

TextureManager::TextureManager() : m_texturesLoaded(true) {
    // Create textures for each tetromino color (1-7)
    for (int colorId = 1; colorId <= 7; ++colorId) {
        sf::Image blockImage = createBlockImage(colorId, 32);
        
        sf::Texture texture;
        if (!texture.loadFromImage(blockImage)) {
            m_texturesLoaded = false;
            continue;
        }
        
        texture.setSmooth(false);  // Pixel-perfect look
        m_textures[colorId] = texture;
    }
}

TextureManager::~TextureManager() {
}

const sf::Texture& TextureManager::getBlockTexture(int colorId) {
    // Return the texture for this color, or create a fallback
    auto it = m_textures.find(colorId);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    // Return a blank texture if color not found
    static sf::Texture fallbackTexture;
    return fallbackTexture;
}

sf::Image TextureManager::createBlockImage(int colorId, int size) {
    // Base colors for each tetromino type
    sf::Color baseColor;
    sf::Color highlightColor;
    sf::Color shadowColor;
    
    switch (colorId) {
        case 1: // I - Cyan
            baseColor = sf::Color(0, 240, 240);
            highlightColor = sf::Color(100, 255, 255);
            shadowColor = sf::Color(0, 140, 140);
            break;
        case 2: // O - Yellow
            baseColor = sf::Color(240, 240, 0);
            highlightColor = sf::Color(255, 255, 100);
            shadowColor = sf::Color(180, 180, 0);
            break;
        case 3: // T - Purple
            baseColor = sf::Color(160, 0, 240);
            highlightColor = sf::Color(200, 100, 255);
            shadowColor = sf::Color(100, 0, 160);
            break;
        case 4: // S - Green
            baseColor = sf::Color(0, 240, 0);
            highlightColor = sf::Color(100, 255, 100);
            shadowColor = sf::Color(0, 160, 0);
            break;
        case 5: // Z - Red
            baseColor = sf::Color(240, 0, 0);
            highlightColor = sf::Color(255, 100, 100);
            shadowColor = sf::Color(160, 0, 0);
            break;
        case 6: // J - Blue
            baseColor = sf::Color(0, 0, 240);
            highlightColor = sf::Color(100, 100, 255);
            shadowColor = sf::Color(0, 0, 160);
            break;
        case 7: // L - Orange
            baseColor = sf::Color(240, 160, 0);
            highlightColor = sf::Color(255, 200, 100);
            shadowColor = sf::Color(160, 100, 0);
            break;
        default:
            baseColor = sf::Color(100, 100, 100);
            highlightColor = sf::Color(150, 150, 150);
            shadowColor = sf::Color(50, 50, 50);
    }
    
    // Create image with base color
    sf::Image image({static_cast<unsigned>(size), static_cast<unsigned>(size)}, baseColor);
    
    // Add beveled edges for 3D effect
    int margin = size / 8;
    
    // Highlight (top-left edges)
    for (int i = 0; i < margin; ++i) {
        for (int j = i; j < size - i; ++j) {
            // Top edge
            image.setPixel({static_cast<unsigned>(j), static_cast<unsigned>(i)}, highlightColor);
            // Left edge
            image.setPixel({static_cast<unsigned>(i), static_cast<unsigned>(j)}, highlightColor);
        }
    }
    
    // Shadow (bottom-right edges)
    for (int i = 0; i < margin; ++i) {
        for (int j = i; j < size - i; ++j) {
            // Bottom edge
            image.setPixel({static_cast<unsigned>(j), static_cast<unsigned>(size - 1 - i)}, shadowColor);
            // Right edge
            image.setPixel({static_cast<unsigned>(size - 1 - i), static_cast<unsigned>(j)}, shadowColor);
        }
    }
    
    return image;
}
