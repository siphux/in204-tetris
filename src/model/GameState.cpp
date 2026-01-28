#include "GameState.h"
#include "GameMode.h"
#include "LevelBasedMode.h"
#include "AIMode.h"
#include <cstdlib> // for rand()
#include <algorithm> // for std::sort, std::greater

//Choice of permutation given by the bag system (shuffling the pieces)
void GameState::refillBag() {
    m_pieceBag = {
        TetrominoType::I, TetrominoType::J, TetrominoType::L,
        TetrominoType::O, TetrominoType::S, TetrominoType::T, TetrominoType::Z
    };
    for (int i = 6; i > 0; i--) {
        int j = rand() % (i + 1);
        std::swap(m_pieceBag[i], m_pieceBag[j]);
    }
        
    m_bagIndex = 0;
}

//Default constructor
GameState::GameState()
    : m_currentPiece(TetrominoType::I),
      m_nextPiece(TetrominoType::I),
      m_x(4), m_y(-1),
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


void GameState::update(float deltaTime) {
    if (m_gameMode) {
        m_gameMode->update(deltaTime, *this);
    }

    if (m_isClearingLines && !m_gameOver) {
        updateClearingAnimation(deltaTime);
        return; 
    }
    

    if (m_gameOver) {
        return;
    }
    
    m_fallTimer += deltaTime;
    float fallSpeed = m_gameMode ? m_gameMode->getFallSpeed() : 0.5f;
    if (m_fallTimer > fallSpeed) {
        softDrop();
        m_fallTimer = 0.f; 
    }
}

void GameState::moveLeft() {
    m_x--;
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        m_x++;
    }
}

void GameState::moveRight() {
    m_x++;
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        m_x--;
    }
}


void GameState::rotateClockwise() {
    RotationState oldState = m_currentPiece.getRotationState();
    m_currentPiece.rotateClockwise();
    RotationState newState = m_currentPiece.getRotationState();
    
    const auto& wallKicks = Tetromino::getWallKicks(
        m_currentPiece.getType(), oldState, newState
    );
    
    bool rotated = false;
    for (const auto& kick : wallKicks) {
        int testX = m_x + kick.x;
        int testY = m_y + kick.y;
        
        if (!m_board.checkCollision(m_currentPiece.getBlocks(), testX, testY)) {
            m_x = testX;
            m_y = testY;
            rotated = true;
            break;
        }
    }
    
    if (!rotated) {
        m_currentPiece.rotateCounterClockwise();
    }
}

void GameState::rotateCounterClockwise() {
    RotationState oldState = m_currentPiece.getRotationState();
    m_currentPiece.rotateCounterClockwise();
    RotationState newState = m_currentPiece.getRotationState();
    
    const auto& wallKicks = Tetromino::getWallKicks(
        m_currentPiece.getType(), oldState, newState
    );
    
    bool rotated = false;
    for (const auto& kick : wallKicks) {
        int testX = m_x + kick.x;
        int testY = m_y + kick.y;
        
        if (!m_board.checkCollision(m_currentPiece.getBlocks(), testX, testY)) {
            m_x = testX;
            m_y = testY;
            rotated = true;
            break;
        }
    }
    if (!rotated) {
        m_currentPiece.rotateClockwise();
    }
}

//move piece down by one row
void GameState::softDrop() {
    m_y++;  
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        m_y--;  
        lockPiece();  
    }
}

//Instant drop piece to the bottom
void GameState::hardDrop() {
    while (!m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y + 1)) {
        m_y++;
    }
    lockPiece();
}

// Lock piece into the board
void GameState::lockPiece() {
    for (const auto& block : m_currentPiece.getBlocks()) {
        int bx = m_x + block.x;
        int by = m_y + block.y;
        if (m_board.isInside(bx, by)) {
            m_board.setCell(bx, by, m_currentPiece.getColorId());
        }
    }
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
        m_isClearingLines = true;
        m_clearAnimationTimer = 0.0f;
        m_linesToClear = fullLines;
        
        for (int y : fullLines) {
            for (int x = 0; x < Board::Width; x++) {
                m_board.setCell(x, y, -1); // We mark the blocks of the line to be cleared
            }
        }
    } else {
        spawnNewPiece();
    }
}


void GameState::updateClearingAnimation(float deltaTime) {
    if (!m_isClearingLines) return;
    
    m_clearAnimationTimer += deltaTime;
    
    if (m_clearAnimationTimer >= CLEAR_ANIMATION_DURATION) {
        int writeY = Board::Height - 1;
        for (int readY = Board::Height - 1; readY >= 0; readY--) {
            if (m_board.getCell(0, readY) == -1) {
                continue;
            }
            if (writeY != readY) {
                for (int x = 0; x < Board::Width; x++) {
                    m_board.setCell(x, writeY, m_board.getCell(x, readY));
                }
            }
            writeY--;
        }

        while (writeY >= 0) {
            for (int x = 0; x < Board::Width; x++) {
                m_board.setCell(x, writeY, 0);
            }
            writeY--;
        }
        int linesCleared = static_cast<int>(m_linesToClear.size());
        int currentLevel = 0;
        if (m_gameMode) {
            const auto* levelMode = dynamic_cast<const LevelBasedMode*>(m_gameMode.get());
            const auto* aiMode = dynamic_cast<const AIMode*>(m_gameMode.get());
            if (levelMode) {
                currentLevel = levelMode->getCurrentLevel();
            } else if (aiMode) {
                currentLevel = aiMode->getCurrentLevel();
            }
            m_gameMode->onLinesClear(linesCleared, *this);
        }
        m_score.addLineClear(linesCleared, currentLevel);
        m_isClearingLines = false;
        m_clearAnimationTimer = 0.0f;
        m_linesToClear.clear();
        
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
    
    if (m_gameMode) {
        m_gameMode->reset();
    }

    refillBag();
    m_bagIndex = 0;

    spawnNewPiece();
}



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
        m_gameOver = true;
        return;
    }
    for (int y = 0; y < Board::Height - numLines; y++) {
        for (int x = 0; x < Board::Width; x++) {
            int sourceY = y + numLines;
            int cellValue = m_board.getCell(x, sourceY);
            m_board.setCell(x, y, cellValue);
        }
    }
    
    for (int line = 0; line < numLines; line++) {
        int garbageY = Board::Height - numLines + line;
        int holeX = rand() % Board::Width;
        
        for (int x = 0; x < Board::Width; x++) {
            if (x == holeX) {
                m_board.setCell(x, garbageY, 0);
            } else {
                m_board.setCell(x, garbageY, 8);
            }
        }
    }
    
    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        while (m_y > 0 && m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
            m_y--;
        }
        
        if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
            m_gameOver = true;
        }
    }
}

//for multiplayer
void GameState::syncBoard(const Board& board) {
    for (int y = 0; y < Board::Height; y++) {
        for (int x = 0; x < Board::Width; x++) {
            m_board.setCell(x, y, board.getCell(x, y));
        }
    }
}

void GameState::syncPiecePosition(int x, int y, int rotation) {
    m_x = x;
    m_y = y;
    m_currentPiece.setRotationState(static_cast<RotationState>(rotation % 4));
}

//spawn a new piece at the top of the board
void GameState::spawnNewPiece() {
    if (m_bagIndex >= m_pieceBag.size()) {
        refillBag();
    }

    m_currentPiece = Tetromino(m_pieceBag[m_bagIndex]);
    m_bagIndex++;

    if (m_bagIndex >= m_pieceBag.size()) {
        refillBag();
    }
    m_nextPiece = Tetromino(m_pieceBag[m_bagIndex]);
    m_x = 4;
    m_y = -1;
        

    if (m_board.checkCollision(m_currentPiece.getBlocks(), m_x, m_y)) {
        m_gameOver = true;
    }
}
