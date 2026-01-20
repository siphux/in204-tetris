#include "GameState.h"
#include <cstdlib> // for rand()

GameState::GameState()
    : m_currentPiece(TetrominoType::O),
      m_x(4), m_y(0),
      m_fallTimer(0.f) {}

// Update game logic
void GameState::update(float deltaTime) {
    m_fallTimer += deltaTime;
    if (m_fallTimer > m_level.fallSpeed()) {
        softDrop(); // piece moves down automatically
        m_fallTimer = 0.f;
    }
}

void GameState::moveLeft() {
    m_x--;
    // TODO: check collision with board (Day 2)
}

void GameState::moveRight() {
    m_x++;
    // TODO: check collision with board (Day 2)
}

// Move piece down by one row
void GameState::softDrop() {
    m_y++;
    // TODO: check collision with board
    // if collision -> lockPiece()
}

// Lock piece into the board
void GameState::lockPiece() {
    // Place piece blocks into board
    for (const auto& block : m_currentPiece.getBlocks()) {
        int bx = m_x + block.x;
        int by = m_y + block.y;
        if (m_board.isInside(bx, by)) {
            m_board.setCell(bx, by, m_currentPiece.getColorId());
        }
    }

    // Clear full lines
    int linesCleared = 0;
    for (int y = 0; y < Board::Height; y++) {
        bool full = true;
        for (int x = 0; x < Board::Width; x++) {
            if (m_board.getCell(x, y) == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            // Shift lines down
            for (int yy = y; yy > 0; yy--) {
                for (int x = 0; x < Board::Width; x++) {
                    m_board.setCell(x, yy, m_board.getCell(x, yy-1));
                }
            }
            // Clear top row
            for (int x = 0; x < Board::Width; x++) m_board.setCell(x, 0, 0);

            linesCleared++;
        }
    }

    // Update score and level
    if (linesCleared > 0) {
        m_score.addLineClear(linesCleared, m_level.current());
        m_level.addLines(linesCleared);
    }

    // Spawn new piece
    spawnNewPiece();
}

// Accessors
const Board& GameState::board() const { return m_board; }
const Tetromino& GameState::currentPiece() const { return m_currentPiece; }
int GameState::pieceX() const { return m_x; }
int GameState::pieceY() const { return m_y; }
int GameState::score() const { return m_score.value(); }
int GameState::level() const { return m_level.current(); }

// Spawn a new random piece at the top
void GameState::spawnNewPiece() {
    TetrominoType t = static_cast<TetrominoType>(rand() % 7);
    m_currentPiece = Tetromino(t);
    m_x = 4;
    m_y = 0;
    // TODO: check for game over (collision at spawn)
}

