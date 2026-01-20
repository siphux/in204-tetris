#include "GameView.h"
#include <algorithm>
#include <cmath>
#include <string>

GameView::GameView() : m_fontLoaded(false) {
    // Essayer de charger une police système ou inclure une police TTF
    // Option 1: Utiliser une police système (Linux)
    if (!m_font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        // Option 2: Essayer une autre police commune
        if (!m_font.openFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
            // Option 3: Police par défaut (peut ne pas fonctionner)
            m_fontLoaded = false;
        } else {
            m_fontLoaded = true;
        }
    } else {
        m_fontLoaded = true;
    }
}

void GameView::render(sf::RenderWindow& window, const GameState& state) {
    window.clear(sf::Color::Black);

    if (state.isGameOver()) {
        drawGameOverScreen(window, state.score());
    } else {
        // Dessiner le board avec animation si nécessaire
        drawBoard(window, state.board(), state);
        
        // Ne pas dessiner la pièce courante pendant l'animation de suppression
        if (!state.isClearingLines()) {
            drawCurrentPiece(window, state);
        }
        
        drawNextPiece(window, state.nextPiece());

        drawUI(window, state);
    }

    window.display();
}

sf::Color GameView::colorForId(int colorId) const {
    switch (colorId) {
        case 1: return sf::Color::Cyan;                 // I
        case 2: return sf::Color::Blue;                 // J
        case 3: return sf::Color(255, 165, 0);          // L (orange)
        case 4: return sf::Color::Yellow;               // O
        case 5: return sf::Color::Green;                // S
        case 6: return sf::Color(128, 0, 128);          // T (purple)
        case 7: return sf::Color::Red;                  // Z
        default: return sf::Color(40, 40, 40);          // empty / unknown
    }
}

void GameView::drawBoard(sf::RenderWindow& window, const Board& board, const GameState& state) {
    sf::RectangleShape cell;
    cell.setSize(sf::Vector2f(static_cast<float>(CellSize - 1),
                              static_cast<float>(CellSize - 1)));

    // Obtenir le progrès de l'animation si une suppression est en cours
    float animationProgress = state.isClearingLines() ? state.getClearAnimationProgress() : 0.0f;

    // Commencer à y=1 pour cacher la ligne de spawn (ligne 0)
    for (int y = 1; y < Board::Height; ++y) {
        for (int x = 0; x < Board::Width; ++x) {
            const int value = board.getCell(x, y);
            
            // Vérifier si c'est une ligne en cours de suppression (valeur -1)
            if (value == -1) {
                // Ligne en cours de suppression - effet visuel avec fade out
                // L'alpha diminue de 255 à 0 pendant l'animation
                // Utiliser un effet de pulsation pour plus de visibilité
                float pulse = 0.5f + 0.5f * std::sin(animationProgress * 3.14159f * 4.0f); // Pulsation rapide
                unsigned char alpha = static_cast<unsigned char>(255 * (1.0f - animationProgress) * pulse);
                cell.setFillColor(sf::Color(255, 255, 255, alpha)); // Blanc qui disparaît avec pulsation
            } else if (value == 0) {
                cell.setFillColor(sf::Color(30, 30, 30)); // Vide
            } else {
                cell.setFillColor(colorForId(value)); // Couleur normale
            }
            
            cell.setPosition({static_cast<float>(BoardOffsetX + x * CellSize),
                              static_cast<float>(BoardOffsetY + (y - 1) * CellSize)});
            window.draw(cell);
        }
    }

    // Board border
    sf::RectangleShape border;
    border.setSize(sf::Vector2f(static_cast<float>(Board::Width * CellSize),
                                static_cast<float>((Board::Height - 1) * CellSize)));
    border.setPosition({static_cast<float>(BoardOffsetX), static_cast<float>(BoardOffsetY)});
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(2.f);
    window.draw(border);
}

void GameView::drawCurrentPiece(sf::RenderWindow& window, const GameState& state) {
    const Tetromino& piece = state.currentPiece();
    const int baseX = state.pieceX();
    const int baseY = state.pieceY();

    sf::RectangleShape block;
    block.setSize(sf::Vector2f(static_cast<float>(CellSize - 1),
                               static_cast<float>(CellSize - 1)));
    block.setFillColor(colorForId(piece.getColorId()));
    // Add outline to current piece for better visibility
    block.setOutlineThickness(1.0f);
    block.setOutlineColor(sf::Color(255, 255, 255, 128));

    // Draw Ghost Piece
    int ghostY = state.getGhostY();
    sf::RectangleShape ghostBlock = block;
    sf::Color ghostColor = block.getFillColor();
    ghostColor.a = 64; // Semi-transparent
    ghostBlock.setFillColor(ghostColor);
    ghostBlock.setOutlineColor(sf::Color(255, 255, 255, 64));

    for (const auto& offset : piece.getBlocks()) {
        const int x = baseX + offset.x;
        const int y = ghostY + offset.y;
        if (x >= 0 && x < Board::Width && y >= 1 && y < Board::Height) {
            ghostBlock.setPosition({static_cast<float>(BoardOffsetX + x * CellSize),
                                    static_cast<float>(BoardOffsetY + (y - 1) * CellSize)});
            window.draw(ghostBlock);
        }
    }

    for (const auto& offset : piece.getBlocks()) {
        const int x = baseX + offset.x;
        const int y = baseY + offset.y;

        if (x < 0 || x >= Board::Width || y < 1 || y >= Board::Height) {
            continue;
        }

        block.setPosition({static_cast<float>(BoardOffsetX + x * CellSize),
                           static_cast<float>(BoardOffsetY + (y - 1) * CellSize)});
        window.draw(block);
    }
}

void GameView::drawNextPiece(sf::RenderWindow& window, const Tetromino& nextPiece) {
    const int previewX = BoardOffsetX + Board::Width * CellSize + 50;
    const int previewY = BoardOffsetY + 50;

    // Label "NEXT"
    if (m_fontLoaded) {
        sf::Text label(m_font, "NEXT");
        label.setCharacterSize(20);
        label.setFillColor(sf::Color::White);
        label.setPosition({static_cast<float>(previewX), static_cast<float>(previewY - 30)});
        window.draw(label);
    }

    // Determine bounding box to center the preview
    int minX = 999, minY = 999, maxX = -999, maxY = -999;
    for (const auto& block : nextPiece.getBlocks()) {
        minX = std::min(minX, block.x);
        minY = std::min(minY, block.y);
        maxX = std::max(maxX, block.x);
        maxY = std::max(maxY, block.y);
    }

    const int width = maxX - minX + 1;
    const int height = maxY - minY + 1;

    const int offsetX = previewX + (4 - width) * CellSize / 2;
    const int offsetY = previewY + (4 - height) * CellSize / 2;

    sf::RectangleShape block;
    block.setSize(sf::Vector2f(static_cast<float>(CellSize - 1),
                               static_cast<float>(CellSize - 1)));
    block.setFillColor(colorForId(nextPiece.getColorId()));

    for (const auto& b : nextPiece.getBlocks()) {
        const int x = b.x - minX;
        const int y = b.y - minY;
        block.setPosition({static_cast<float>(offsetX + x * CellSize),
                           static_cast<float>(offsetY + y * CellSize)});
        window.draw(block);
    }
}

void GameView::drawUI(sf::RenderWindow& window, const GameState& state) {
    if (!m_fontLoaded) return; // Pas de police, pas d'UI texte

    const float uiX = BoardOffsetX + Board::Width * CellSize + 50.0f;
    float uiY = BoardOffsetY + 200.0f;
    const float lineHeight = 30.0f;

    // Score
    sf::Text scoreText(m_font, "Score: " + std::to_string(state.score()));
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({uiX, uiY});
    window.draw(scoreText);

    // Level
    uiY += lineHeight;
    sf::Text levelText(m_font, "Level: " + std::to_string(state.level()));
    levelText.setCharacterSize(24);
    levelText.setFillColor(sf::Color::White);
    levelText.setPosition({uiX, uiY});
    window.draw(levelText);
}

void GameView::drawGameOverScreen(sf::RenderWindow& window, int finalScore) {
    // Fond semi-transparent
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 180)); // Noir semi-transparent
    window.draw(overlay);

    if (!m_fontLoaded) return;

    // Texte "GAME OVER"
    sf::Text gameOverText(m_font, "GAME OVER");
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setStyle(sf::Text::Bold);

    // Centrer le texte
    sf::FloatRect bounds = gameOverText.getLocalBounds();
    gameOverText.setPosition({
        (window.getSize().x - bounds.size.x) / 2.0f,
        window.getSize().y / 2.0f - 50.0f
    });
    window.draw(gameOverText);

    // Score final
    sf::Text scoreText(m_font, "Final Score: " + std::to_string(finalScore));
    scoreText.setCharacterSize(32);
    scoreText.setFillColor(sf::Color::White);

    bounds = scoreText.getLocalBounds();
    scoreText.setPosition({
        (window.getSize().x - bounds.size.x) / 2.0f,
        window.getSize().y / 2.0f + 20.0f
    });
    window.draw(scoreText);

    // Instructions
    sf::Text restartText(m_font, "Press R to Restart");
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::Yellow);

    bounds = restartText.getLocalBounds();
    restartText.setPosition({
        (window.getSize().x - bounds.size.x) / 2.0f,
        window.getSize().y / 2.0f + 80.0f
    });
    window.draw(restartText);
}