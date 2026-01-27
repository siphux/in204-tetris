#include "GameState.h"
#include "GameMode.h"
#include "LevelBasedMode.h"
#include "AIMode.h"
#include <cstdlib> // for rand()
#include <algorithm> // for std::sort, std::greater

// Refill the piece bag with all 7 tetromino types
// Uses "bag system" - ensures you get all 7 pieces before getting repeats
void GameState::refillBag() {
    // Put all 7 piece types in the bag
    m_pieceBag = {
        TetrominoType::I, TetrominoType::J, TetrominoType::L,
        TetrominoType::O, TetrominoType::S, TetrominoType::T, TetrominoType::Z
    };
        
    // Shuffle the bag (Fisher-Yates shuffle algorithm)
    // This randomizes the order so pieces come in random sequence
    for (int i = 6; i > 0; i--) {
        int j = rand() % (i + 1);  // Pick a random position
        std::swap(m_pieceBag[i], m_pieceBag[j]);  // Swap pieces
    }
        
    m_bagIndex = 0;  // Start from the beginning of the bag
}

GameState::GameState()
    : m_currentPiece(TetrominoType::I), // Placeholder, will be replaced
      m_nextPiece(TetrominoType::I),  // Placeholder, will be replaced
      m_x(4), m_y(-1),  // Spawn in hidden row
      m_fallTimer(0.f),
      m_isClearingLines(false),
      m_clearAnimationTimer(0.0f),
      m_gameOver(false),
      m_gameMode(std::make_unique<LevelBasedMode>())
{
    refillBag();
    spawnNewPiece();
}

GameState::~GameState() = default;

// Update game logic every frame
// This makes pieces fall, checks for line clears, spawns new pieces, etc.
void GameState::update(float deltaTime) {
    // Update mode-specific logic (level progression, speed changes, etc.)
    if (m_gameMode) {
        m_gameMode->update(deltaTime, *this);
    }

    // If we're clearing lines, show the animation instead of falling
    if (m_isClearingLines && !m_gameOver) {
        updateClearingAnimation(deltaTime);
        return;  // Don't make the piece fall during line clearing animation
    }
    
    // If game is over, don't update anything
    if (m_gameOver) {
        return;
    }
    
    // Make the piece fall automatically
    m_fallTimer += deltaTime;
    float fallSpeed = m_gameMode ? m_gameMode->getFallSpeed() : 0.5f;  // Get fall speed from game mode
    if (m_fallTimer > fallSpeed) {
        softDrop();  // Move piece down one row
        m_fallTimer = 0.f;  // Reset timer
    }
}

// Move the current piece one space to the left
void GameState::moveLeft() {
    m_x--;  // Try moving left
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        // Collision detected - can't move here, so move back
        m_x++;
    }
}

// Move the current piece one space to the right
void GameState::moveRight() {
    m_x++;  // Try moving right
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        // Collision detected - can't move here, so move back
        m_x--;
    }
}

// Rotate the piece clockwise (to the right)
// Uses "wall kicks" - if rotation would hit a wall, try moving the piece slightly
void GameState::rotateClockwise() {
    // Remember the rotation state before rotating
    RotationState oldState = m_currentPiece.getRotationState();
    m_currentPiece.rotateClockwise();  // Try to rotate
    RotationState newState = m_currentPiece.getRotationState();
    
    // Get wall kick offsets - these are small position adjustments to try
    // if the rotation would normally hit a wall
    const auto& wallKicks = Tetromino::getWallKicks(
        m_currentPiece.getType(), oldState, newState
    );
    
    // Try each wall kick offset to see if any of them work
    bool rotated = false;
    for (const auto& kick : wallKicks) {
        int testX = m_x + kick.x;  // Try moving piece slightly
        int testY = m_y + kick.y;
        
        // Check if this position works (no collision)
        if (!m_board.checkCollision(m_currentPiece.getBlocks(), testX, testY)) {
            // This offset works! Use it
            m_x = testX;
            m_y = testY;
            rotated = true;
            break;  // Found a working position, stop trying
        }
    }
    
    // If no wall kick worked, the rotation is impossible - revert it
    if (!rotated) {
        m_currentPiece.rotateCounterClockwise();  // Undo the rotation
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

// Move piece down by one row (called automatically or when holding down arrow)
void GameState::softDrop() {
    m_y++;  // Try moving down
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        // Collision detected - piece can't fall further
        m_y--;  // Move back up to the last valid position
        lockPiece();  // Lock the piece into the board (it can't move anymore)
    }
}

// Drop piece instantly to the bottom (when player presses space)
void GameState::hardDrop() {
    // Keep moving down until we hit something (board or bottom)
    while (!m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y + 1)) {
        m_y++;  // Move down one more row
    }
    // Piece is now at the bottom - lock it immediately
    lockPiece();
}

// Lock piece into the board (piece can't move anymore)
// This happens when the piece hits something below it
void GameState::lockPiece() {
    // Place each block of the piece into the board
    for (const auto& block : m_currentPiece.getBlocks()) {
        // Calculate the board position of this block
        int bx = m_x + block.x;
        int by = m_y + block.y;
        
        // Make sure the position is valid, then place the block
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

// Update the line clearing animation
// Shows a visual effect before actually removing the lines
void GameState::updateClearingAnimation(float deltaTime) {
    if (!m_isClearingLines) return;
    
    m_clearAnimationTimer += deltaTime;
    
    // When animation is done, actually remove the lines
    if (m_clearAnimationTimer >= CLEAR_ANIMATION_DURATION) {
        // Animation finished - now remove the lines
        // Rebuild the board by compacting lines (remove cleared lines, make others fall down)
        int writeY = Board::Height - 1;  // Where we'll write the next line

        // Go through each row from bottom to top
        for (int readY = Board::Height - 1; readY >= 0; readY--) {
            // Check if this line is marked for deletion (-1 means "being cleared")
            // We check the first cell - if it's -1, the whole line is marked
            if (m_board.getCell(0, readY) == -1) {
                continue;  // Skip this line - it's being deleted
            }

            // This line is not being deleted - move it down to fill the gap
            if (writeY != readY) {
                // Copy the entire row
                for (int x = 0; x < Board::Width; x++) {
                    m_board.setCell(x, writeY, m_board.getCell(x, readY));
                }
            }
            writeY--;  // Move to the next position to write
        }

        // Fill the remaining top rows with empty cells (they fell down)
        while (writeY >= 0) {
            for (int x = 0; x < Board::Width; x++) {
                m_board.setCell(x, writeY, 0);  // 0 = empty cell
            }
            writeY--;
        }
        
        // Update score and game mode
        int linesCleared = static_cast<int>(m_linesToClear.size());
        int currentLevel = 0;
        if (m_gameMode) {
            // Get the current level from the game mode (for scoring)
            // Different modes (LevelBased, AI) track levels differently
            const auto* levelMode = dynamic_cast<const LevelBasedMode*>(m_gameMode.get());
            const auto* aiMode = dynamic_cast<const AIMode*>(m_gameMode.get());
            if (levelMode) {
                currentLevel = levelMode->getCurrentLevel();
            } else if (aiMode) {
                currentLevel = aiMode->getCurrentLevel();
            }
            // Tell the game mode that lines were cleared (for level progression, etc.)
            m_gameMode->onLinesClear(linesCleared, *this);
        }
        // Add points to the score based on how many lines were cleared and current level
        m_score.addLineClear(linesCleared, currentLevel);
        
        // Réinitialiser l'état
        m_isClearingLines = false;
        m_clearAnimationTimer = 0.0f;
        m_linesToClear.clear();
        
        // Spawner nouvelle pièce seulement si le jeu n'est pas terminé
        if (!m_gameOver) {
            spawnNewPiece();
        }
    }
}

void GameState::reset() {
    m_board.clear();
    m_score.reset();
    m_gameOver = false;
    m_isClearingLines = false;
    m_clearAnimationTimer = 0.0f;
    m_linesToClear.clear();
    
    // Reset game mode
    if (m_gameMode) {
        m_gameMode->reset();
    }

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
int GameState::level() const {
    if (!m_gameMode) return 0;
    
    // Get level from mode if it supports it
    const auto* levelMode = dynamic_cast<const LevelBasedMode*>(m_gameMode.get());
    const auto* aiMode = dynamic_cast<const AIMode*>(m_gameMode.get());
    if (levelMode) {
        return levelMode->getCurrentLevel();
    } else if (aiMode) {
        return aiMode->getCurrentLevel();
    }
    return 0;
}

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

void GameState::setGameMode(std::unique_ptr<GameMode> mode) {
    m_gameMode = std::move(mode);
    reset();
}

GameMode* GameState::getGameMode() const {
    return m_gameMode.get();
}

void GameState::addGarbageLines(int numLines) {
    if (numLines <= 0) return;
    if (numLines >= Board::Height) {
        // Too many lines, game over
        m_gameOver = true;
        return;
    }
    
    // Shift all existing blocks up by numLines
    // Copy from bottom to top
    for (int y = 0; y < Board::Height - numLines; y++) {
        for (int x = 0; x < Board::Width; x++) {
            int sourceY = y + numLines;
            int cellValue = m_board.getCell(x, sourceY);
            m_board.setCell(x, y, cellValue);
        }
    }
    
    // Add garbage lines at the bottom with random holes (1 hole per line)
    for (int line = 0; line < numLines; line++) {
        int garbageY = Board::Height - numLines + line;
        int holeX = rand() % Board::Width;  // Random hole position
        
        for (int x = 0; x < Board::Width; x++) {
            if (x == holeX) {
                // Leave one hole per line
                m_board.setCell(x, garbageY, 0);
            } else {
                // Fill with garbage (use color ID 8 for garbage)
                m_board.setCell(x, garbageY, 8);
            }
        }
    }
    
    // Check if piece is now in an invalid position
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        // Piece is colliding, try to move it up
        while (m_y > 0 && m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
            m_y--;
        }
        
        // If still colliding, game over
        if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
            m_gameOver = true;
        }
    }
}

// Copy another board's state into this board
// Used in multiplayer to sync the remote player's board
void GameState::syncBoard(const Board& board) {
    // Copy every cell from the other board
    for (int y = 0; y < Board::Height; y++) {
        for (int x = 0; x < Board::Width; x++) {
            m_board.setCell(x, y, board.getCell(x, y));
        }
    }
}

void GameState::syncPiecePosition(int x, int y, int rotation) {
    // Sync opponent's falling piece position for display
    // This updates the current piece's position and rotation state
    m_x = x;
    m_y = y;
    
    // Apply rotation state (rotation should be 0-3)
    m_currentPiece.setRotationState(static_cast<RotationState>(rotation % 4));
}

// Spawn a new random piece at the top of the board
// Uses the "bag system" to ensure fair piece distribution
void GameState::spawnNewPiece() {
    // Check if the bag is empty (we've used all pieces)
    if (m_bagIndex >= m_pieceBag.size()) {
        refillBag();  // Get a new bag of all 7 pieces
    }
        
    // Take the next piece from the bag
    m_currentPiece = Tetromino(m_pieceBag[m_bagIndex]);
    m_bagIndex++;  // Move to next piece in bag
        
    // Generate the next piece for the preview (so player can see what's coming)
    if (m_bagIndex >= m_pieceBag.size()) {
        refillBag();  // If bag is empty, refill it
    }
    m_nextPiece = Tetromino(m_pieceBag[m_bagIndex]);  // This is what will spawn next
        
    // Position the new piece at the top center of the board
    // Spawn at y = -1 to account for pieces that extend above their spawn point
    // (e.g., J and L pieces have blocks at relative y = -1)
    // This puts pieces in the hidden spawn row (row 0 is the first visible row)
    m_x = 4;  // Center horizontally (board is 10 wide, so 4 is center)
    m_y = -1;  // Spawn in hidden row (above visible board at row 0)
        
    // Check if the new piece collides immediately (game over condition)
    // The checkCollision function allows y < 0 (above board), so this should work correctly
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        m_gameOver = true;  // Can't spawn - game over!
    }
}
