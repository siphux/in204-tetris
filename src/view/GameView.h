#pragma once
#include <SFML/Graphics.hpp>
#include "../model/GameState.h"
#include "../model/GameMode.h"
#include "MenuView.h"
#include "TextureManager.h"
#include <memory>

// Handles rendering the game state to the window.
class GameView {
public:
    GameView();

    // Render the entire game scene (game or menu)
    void render(sf::RenderWindow& window, const GameState& state, 
               const MenuView& menuView, MenuState menuState, int selectedOption,
               bool isMultiplayer = false, const GameState* remoteState = nullptr,
               int winnerId = -1, const std::string& winnerName = "",
               bool isNetworkConnected = false, const std::string& localIP = "",
               bool localPlayerReady = false, bool remotePlayerReady = false,
               float musicVolume = 50.0f);

    // Render just the game (without menu overlay)
    void renderGame(sf::RenderWindow& window, const GameState& state, 
                   bool isMultiplayer = false, const GameState* remoteState = nullptr);
    
    // Set IP input for JOIN_GAME menu
    void setIPInput(const std::string& ipInput) { m_ipInput = ipInput; }
    
    // Set ready status for NETWORK_READY menu
    void setNetworkReadyStatus(bool localReady, bool remoteReady) {
        m_localPlayerReady = localReady;
        m_remotePlayerReady = remoteReady;
    }

private:
    static constexpr int CellSize = 30;
    static constexpr int BoardOffsetX = 50;
    static constexpr int BoardOffsetY = 50;

    sf::Font m_font;
    bool m_fontLoaded;
    std::string m_ipInput;  // For JOIN_GAME menu
    bool m_localPlayerReady;  // For NETWORK_READY menu
    bool m_remotePlayerReady;  // For NETWORK_READY menu
    std::unique_ptr<TextureManager> m_textureManager;  // For block textures

    sf::Color colorForId(int colorId) const;

    void drawBoard(sf::RenderWindow& window, const Board& board, const GameState& state, 
                   float offsetX = 0.0f, float offsetY = 0.0f);
    void drawCurrentPiece(sf::RenderWindow& window, const GameState& state, 
                          float offsetX = 0.0f, float offsetY = 0.0f);
    void drawNextPiece(sf::RenderWindow& window, const Tetromino& nextPiece, 
                       float offsetX = 0.0f, float offsetY = 0.0f);
    void drawUI(sf::RenderWindow& window, const GameState& state, 
                float offsetX = 0.0f, const std::string& playerLabel = "");
    void drawGameOverScreen(sf::RenderWindow& window, int finalScore);
};
