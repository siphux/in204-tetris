#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>

// Manages block textures for Tetris
class TextureManager {
public:
    TextureManager();
    ~TextureManager();
    
    const sf::Texture& getBlockTexture(int colorId);
    
    bool isLoaded() const { return m_texturesLoaded; }
    
private:
    std::map<int, sf::Texture> m_textures;
    bool m_texturesLoaded;
    
    sf::Image createBlockImage(int colorId, int size = 32);
};