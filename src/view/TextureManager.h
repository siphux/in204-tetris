#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>

// Manages block textures for Tetris
// Creates textured blocks for each tetromino color
class TextureManager {
public:
    TextureManager();
    ~TextureManager();
    
    // Get texture for a specific color ID (1-7 for tetrominoes)
    const sf::Texture& getBlockTexture(int colorId);
    
    // Check if textures loaded successfully
    bool isLoaded() const { return m_texturesLoaded; }
    
private:
    std::map<int, sf::Texture> m_textures;
    bool m_texturesLoaded;
    
    // Create a textured block image in memory
    sf::Image createBlockImage(int colorId, int size = 32);
};
