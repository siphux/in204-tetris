#include "GameView.h"
#include <algorithm>

GameView::GameView() = default;

void GameView::render(sf::RenderWindow& window, const GameState& state) {
    window.clear(sf::Color::Black);

    drawBoard(window, state.board());
    drawCurrentPiece(window, state);
    drawNextPiece(window, state.nextPiece());

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

void GameView::drawBoard(sf::RenderWindow& window, const Board& board) {
    sf::RectangleShape cell;
    cell.setSize(sf::Vector2f(static_cast<float>(CellSize - 1),
                              static_cast<float>(CellSize - 1)));

    for (int y = 0; y < Board::Height; ++y) {
        for (int x = 0; x < Board::Width; ++x) {
            const int value = board.getCell(x, y);
            cell.setFillColor(value == 0 ? sf::Color(30, 30, 30) : colorForId(value));
            cell.setPosition({static_cast<float>(BoardOffsetX + x * CellSize),
                              static_cast<float>(BoardOffsetY + y * CellSize)});
            window.draw(cell);
        }
    }

    // Board border
    sf::RectangleShape border;
    border.setSize(sf::Vector2f(static_cast<float>(Board::Width * CellSize),
                                static_cast<float>(Board::Height * CellSize)));
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

    for (const auto& offset : piece.getBlocks()) {
        const int x = baseX + offset.x;
        const int y = baseY + offset.y;

        if (x < 0 || x >= Board::Width || y < 0 || y >= Board::Height) {
            continue;
        }

        block.setPosition({static_cast<float>(BoardOffsetX + x * CellSize),
                           static_cast<float>(BoardOffsetY + y * CellSize)});
        window.draw(block);
    }
}

void GameView::drawNextPiece(sf::RenderWindow& window, const Tetromino& nextPiece) {
    const int previewX = BoardOffsetX + Board::Width * CellSize + 50;
    const int previewY = BoardOffsetY + 50;

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
