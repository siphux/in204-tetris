#include "GameState.h"
#include <cstdlib> // for rand()
#include <algorithm> // for std::sort, std::greater

void GameState::refillBag() {
    m_pieceBag = {
        TetrominoType::I, TetrominoType::J, TetrominoType::L,
        TetrominoType::O, TetrominoType::S, TetrominoType::T, TetrominoType::Z
    };
        
    // Mélanger (Fisher-Yates shuffle)
    for (int i = 6; i > 0; i--) {
        int j = rand() % (i + 1);
        std::swap(m_pieceBag[i], m_pieceBag[j]);
    }
        
    m_bagIndex = 0;
}

GameState::GameState()
    : m_currentPiece(TetrominoType::I), // Placeholder, will be replaced
      m_nextPiece(TetrominoType::I),  // Placeholder, will be replaced
      m_x(4), m_y(0),
      m_fallTimer(0.f),
      m_isClearingLines(false),
      m_clearAnimationTimer(0.0f),
      m_gameOver(false)
{
    refillBag();
    spawnNewPiece();
}

// Update game logic
void GameState::update(float deltaTime) {
    if (m_isClearingLines) {
        updateClearingAnimation(deltaTime);
        return; // Ne pas faire tomber la pièce pendant l'animation
    }
    
    m_fallTimer += deltaTime;
    if (m_fallTimer > m_level.fallSpeed()) {
        softDrop(); // piece moves down automatically
        m_fallTimer = 0.f;
    }
}

void GameState::moveLeft() {
    m_x--;
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        // Collision - revert move
        m_x++;
    }
}

void GameState::moveRight() {
    m_x++;
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        // Collision - revert move
        m_x--;
    }
}

void GameState::rotateClockwise() {
    RotationState oldState = m_currentPiece.getRotationState();
    m_currentPiece.rotateClockwise();
    RotationState newState = m_currentPiece.getRotationState();
    
    // Get wall kick offsets for this rotation
    const auto& wallKicks = Tetromino::getWallKicks(
        m_currentPiece.getType(), oldState, newState
    );
    
    // Try each wall kick offset
    bool rotated = false;
    for (const auto& kick : wallKicks) {
        int testX = m_x + kick.x;
        int testY = m_y + kick.y;
        
        if (!m_board.checkCollision(m_currentPiece.getBlocks(), testX, testY)) {
            // This offset works!
            m_x = testX;
            m_y = testY;
            rotated = true;
            break;
        }
    }
    
    // If no wall kick worked, revert rotation
    if (!rotated) {
        m_currentPiece.rotateCounterClockwise();
    }
}

void GameState::rotateCounterClockwise() {
    RotationState oldState = m_currentPiece.getRotationState();
    m_currentPiece.rotateCounterClockwise();
    RotationState newState = m_currentPiece.getRotationState();
    
    // Get wall kick offsets for this rotation
    const auto& wallKicks = Tetromino::getWallKicks(
        m_currentPiece.getType(), oldState, newState
    );
    
    // Try each wall kick offset
    bool rotated = false;
    for (const auto& kick : wallKicks) {
        int testX = m_x + kick.x;
        int testY = m_y + kick.y;
        
        if (!m_board.checkCollision(m_currentPiece.getBlocks(), testX, testY)) {
            // This offset works!
            m_x = testX;
            m_y = testY;
            rotated = true;
            break;
        }
    }
    
    // If no wall kick worked, revert rotation
    if (!rotated) {
        m_currentPiece.rotateClockwise();
    }
}

// Move piece down by one row
void GameState::softDrop() {
    m_y++;
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        // Collision - piece can't fall further
        m_y--; // Move back up
        lockPiece(); // Lock the piece into the board
    }
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
    std::vector<int> fullLines;
    for (int y = Board::Height - 1; y >= 0; y--) {
        bool full = true;
        for (int x = 0; x < Board::Width; x++) {
            if (m_board.getCell(x, y) == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            fullLines.push_back(y);
        }
    }
    
    if (!fullLines.empty()) {
        // Démarrer l'animation au lieu de supprimer immédiatement
        m_isClearingLines = true;
        m_clearAnimationTimer = 0.0f;
        m_linesToClear = fullLines;
        
        // Marquer les lignes pour l'animation (utiliser une valeur spéciale, ex: -1)
        for (int y : fullLines) {
            for (int x = 0; x < Board::Width; x++) {
                m_board.setCell(x, y, -1); // Valeur spéciale pour "en cours de suppression"
            }
        }
    } else {
        // Pas de lignes à supprimer, spawner directement
        spawnNewPiece();
    }
}

void GameState::updateClearingAnimation(float deltaTime) {
    if (!m_isClearingLines) return;
    
    m_clearAnimationTimer += deltaTime;
    
    if (m_clearAnimationTimer >= CLEAR_ANIMATION_DURATION) {
        // Animation terminée, supprimer les lignes
        // Reconstruire le board en compactant les lignes (suppression + gravité)
        int writeY = Board::Height - 1;

        for (int readY = Board::Height - 1; readY >= 0; readY--) {
            // Vérifier si la ligne est marquée pour suppression (-1)
            // On vérifie la première cellule (toute la ligne est à -1 si marquée)
            if (m_board.getCell(0, readY) == -1) {
                continue; // On saute cette ligne (elle est supprimée)
            }

            // Si ce n'est pas une ligne à supprimer, on la déplace vers le bas (writeY)
            if (writeY != readY) {
                for (int x = 0; x < Board::Width; x++) {
                    m_board.setCell(x, writeY, m_board.getCell(x, readY));
                }
            }
            writeY--;
        }

        // Remplir le reste du haut avec des lignes vides
        while (writeY >= 0) {
            for (int x = 0; x < Board::Width; x++) {
                m_board.setCell(x, writeY, 0);
            }
            writeY--;
        }
        
        // Mettre à jour score et level
        int linesCleared = static_cast<int>(m_linesToClear.size());
        m_score.addLineClear(linesCleared, m_level.current());
        m_level.addLines(linesCleared);
        
        // Réinitialiser l'état
        m_isClearingLines = false;
        m_clearAnimationTimer = 0.0f;
        m_linesToClear.clear();
        
        // Spawner nouvelle pièce
        spawnNewPiece();
    }
}

void GameState::reset() {
    m_board.clear();
    m_score.reset();
    m_level = Level(); // Réinitialiser le level
    m_gameOver = false;
    m_isClearingLines = false;
    m_clearAnimationTimer = 0.0f;
    m_linesToClear.clear();

    // Réinitialiser le sac de pièces
    refillBag();
    m_bagIndex = 0;

    // Spawner la première pièce
    spawnNewPiece();
}



// Accessors
const Board& GameState::board() const { return m_board; }
const Tetromino& GameState::currentPiece() const { return m_currentPiece; }
const Tetromino& GameState::nextPiece() const { return m_nextPiece; }
int GameState::pieceX() const { return m_x; }
int GameState::pieceY() const { return m_y; }
int GameState::getGhostY() const {
    int ghostY = m_y;
    while (!m_board.checkCollision(m_currentPiece.getBlocks(), m_x, ghostY + 1)) {
        ghostY++;
    }
    return ghostY;
}
int GameState::score() const { return m_score.value(); }
int GameState::level() const { return m_level.current(); }

bool GameState::isGameOver() const {
    return m_gameOver;
}

bool GameState::isClearingLines() const {
    return m_isClearingLines;
}

float GameState::getClearAnimationProgress() const {
    if (!m_isClearingLines) return 0.0f;
    return m_clearAnimationTimer / CLEAR_ANIMATION_DURATION;
}

// Spawn a new random piece at the top
void GameState::spawnNewPiece() {
    // Vérifier si le sac est vide
    if (m_bagIndex >= m_pieceBag.size()) {
        refillBag();
    }
        
    // Prendre la prochaine pièce du sac
    m_currentPiece = Tetromino(m_pieceBag[m_bagIndex]);
    m_bagIndex++;
        
    // Générer la prochaine pièce pour le preview
    if (m_bagIndex >= m_pieceBag.size()) {
        refillBag();
    }
    m_nextPiece = Tetromino(m_pieceBag[m_bagIndex]);
        
    m_x = 4;
    m_y = 0;
        
    // Vérifier collision au spawn
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        m_gameOver = true;
    }
}
