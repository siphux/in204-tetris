#pragma once
#include <SFML/Graphics.hpp>
#include "../model/GameState.h"

// Handles rendering the game state to the window.
class GameView {
public:
    GameView();

    // Render the entire game scene
    void render(sf::RenderWindow& window, const GameState& state);

private:
    static constexpr int CellSize = 30;
    static constexpr int BoardOffsetX = 50;
    static constexpr int BoardOffsetY = 50;

    sf::Font m_font;
    bool m_fontLoaded;

    sf::Color colorForId(int colorId) const;

    void drawBoard(sf::RenderWindow& window, const Board& board, const GameState& state);
    void drawCurrentPiece(sf::RenderWindow& window, const GameState& state);
    void drawNextPiece(sf::RenderWindow& window, const Tetromino& nextPiece);
    void drawUI(sf::RenderWindow& window, const GameState& state);
    void drawGameOverScreen(sf::RenderWindow& window, int finalScore);
};
