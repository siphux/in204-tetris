#include "GameView.h"
#include "../model/LevelBasedMode.h"
#include "../model/AIMode.h"
#include <algorithm>
#include <cmath>
#include <string>

// GameView: Handles all rendering (drawing) of the game
// This includes the board, pieces, UI, menus, etc.

// Initialize the view - try to load a font for text rendering
GameView::GameView() : m_fontLoaded(false), m_localPlayerReady(false), m_remotePlayerReady(false) {
    // Initialize texture manager for block textures
    m_textureManager = std::make_unique<TextureManager>();
    
    // Try to load a system font for displaying text
    // Option 1: Try DejaVu Sans (common on Linux)
    if (!m_font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        // Option 2: Try Liberation Sans (another common Linux font)
        if (!m_font.openFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
            // Couldn't load any font - text might not display correctly
            m_fontLoaded = false;
        } else {
            m_fontLoaded = true;  // Successfully loaded Liberation Sans
        }
    } else {
        m_fontLoaded = true;  // Successfully loaded DejaVu Sans
    }
}

// Main render function - draws everything to the screen
// Called every frame to update what the player sees
void GameView::render(sf::RenderWindow& window, const GameState& state,
                      const MenuView& menuView, MenuState menuState, int selectedOption,
                      bool isMultiplayer, const GameState* remoteState,
                      int winnerId, const std::string& winnerName,
                      bool isNetworkConnected, const std::string& localIP,
                      bool localPlayerReady, bool remotePlayerReady, float musicVolume) {
    // Store ready status for NETWORK_READY menu rendering
    m_localPlayerReady = localPlayerReady;
    m_remotePlayerReady = remotePlayerReady;
    
    // Clear the window with black background
    window.clear(sf::Color::Black);

    // Always render the game in the background (even if menu is showing)
    renderGame(window, state, isMultiplayer, remoteState);

    // Then render menu on top if a menu is active
    if (menuState != MenuState::NONE) {
        switch (menuState) {
            case MenuState::MAIN_MENU:
                menuView.renderMainMenu(window, selectedOption);
                break;
            case MenuState::MODE_SELECTION:
                menuView.renderModeSelection(window, selectedOption);
                break;
            case MenuState::AI_SELECTION:
                menuView.renderAISelection(window, selectedOption);
                break;
            case MenuState::MULTIPLAYER_MENU:
                menuView.renderMultiplayerMenu(window, selectedOption);
                break;
            case MenuState::LOCAL_MULTIPLAYER:
                menuView.renderLocalMultiplayerMenu(window, selectedOption);
                break;
            case MenuState::LAN_MULTIPLAYER:
                menuView.renderLANMultiplayerMenu(window, selectedOption);
                break;
            case MenuState::HOST_GAME:
                menuView.renderHostGame(window, localIP, isNetworkConnected);
                break;
            case MenuState::JOIN_GAME:
                menuView.renderJoinGame(window, selectedOption, m_ipInput);
                break;
            case MenuState::NETWORK_READY:
                menuView.renderNetworkReady(window, selectedOption, m_localPlayerReady, m_remotePlayerReady);
                break;
            case MenuState::PAUSE_MENU:
                menuView.renderPauseMenu(window, selectedOption, musicVolume);
                break;
            case MenuState::SETTINGS_MENU:
                menuView.renderSettingsMenu(window, selectedOption, musicVolume);
                break;
            case MenuState::GAME_OVER: {
                int player1Lines = 0;
                if (state.getGameMode()) {
                    player1Lines = state.getGameMode()->getLinesCleared();
                }

                int player2Lines = 0;
                int player2Score = 0;
                if (remoteState && remoteState->getGameMode()) {
                    player2Lines = remoteState->getGameMode()->getLinesCleared();
                    player2Score = remoteState->score();
                }

                menuView.renderGameOver(window, state.score(), player1Lines, selectedOption,
                                       isMultiplayer, winnerId, winnerName,
                                       player2Score, player2Lines);
                break;
            }
            default:
                break;
        }
    }

    window.display();
}

// Render the actual game (board, pieces, UI)
// Handles both single-player and multiplayer (split-screen) modes
void GameView::renderGame(sf::RenderWindow& window, const GameState& state, 
                          bool isMultiplayer, const GameState* remoteState) {
    // Game over is handled by the menu system, not here
    
    // Check if we're in multiplayer mode with a remote player
    if (isMultiplayer && remoteState) {
        // Split-screen multiplayer mode - show both players side by side
        float windowWidth = static_cast<float>(window.getSize().x);
        float windowHeight = static_cast<float>(window.getSize().y);
        float boardWidth = Board::Width * CellSize;  // Width of one board in pixels
        float boardHeight = (Board::Height - 1) * CellSize;  // Height of board in pixels
        float spacing = 20.0f;  // Space between the two boards
        float totalWidth = boardWidth * 2 + spacing;  // Total width needed
        float startX = (windowWidth - totalWidth) / 2.0f;  // Center the boards
        
        // Left board (local player)
        float leftX = startX - BoardOffsetX;
        drawBoard(window, state.board(), state, leftX, 0.0f);
        if (!state.isClearingLines()) {
            drawCurrentPiece(window, state, leftX, 0.0f);
        }
        // Preview centered above the board for the left player
        float previewLeftX = leftX + (Board::Width * CellSize - 4 * CellSize) / 2.0f;
        drawNextPiece(window, state.nextPiece(), previewLeftX - 250, 0.0f);
        drawUI(window, state, leftX - BoardOffsetX - 150, "You");
        
        // Right board (remote player)
        float rightX = startX + boardWidth + spacing - BoardOffsetX;
        drawBoard(window, remoteState->board(), *remoteState, rightX, 0.0f);
        if (!remoteState->isClearingLines()) {
            drawCurrentPiece(window, *remoteState, rightX, 0.0f);
        }
        // Preview centered above the board for the right player
        float previewRightX = rightX + (Board::Width * CellSize - 4 * CellSize) / 2.0f;
        drawNextPiece(window, remoteState->nextPiece(), previewRightX + 250, 0.0f);
        drawUI(window, *remoteState, rightX + BoardOffsetX + 300, "Opponent");
        
    } else {
        // Solo mode - normal rendering
        drawBoard(window, state.board(), state);
        
        if (!state.isClearingLines()) {
            drawCurrentPiece(window, state);
        }
        
        // Next piece preview to the right of the board (solo positioning)
        float nextPieceX = BoardOffsetX + Board::Width * CellSize + 50.0f;
        drawNextPiece(window, state.nextPiece(), nextPieceX - BoardOffsetX, 0.0f);
        drawUI(window, state);
    }
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

void GameView::drawBoard(sf::RenderWindow& window, const Board& board, const GameState& state,
                         float offsetX, float offsetY) {
    sf::RectangleShape cell;
    cell.setSize(sf::Vector2f(static_cast<float>(CellSize - 1),
                              static_cast<float>(CellSize - 1)));

    float boardX = BoardOffsetX + offsetX;
    float boardY = BoardOffsetY + offsetY;

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
                cell.setPosition({boardX + static_cast<float>(x * CellSize),
                                  boardY + static_cast<float>((y - 1) * CellSize)});
                window.draw(cell);
            } else if (value == 0) {
                cell.setFillColor(sf::Color(30, 30, 30)); // Vide
                cell.setPosition({boardX + static_cast<float>(x * CellSize),
                                  boardY + static_cast<float>((y - 1) * CellSize)});
                window.draw(cell);
            } else {
                // Draw textured block
                sf::Sprite blockSprite(m_textureManager->getBlockTexture(value));
                blockSprite.setScale({static_cast<float>(CellSize - 1) / 32.0f,
                                    static_cast<float>(CellSize - 1) / 32.0f});
                blockSprite.setPosition({boardX + static_cast<float>(x * CellSize),
                                        boardY + static_cast<float>((y - 1) * CellSize)});
                window.draw(blockSprite);
            }
        }
    }

    // Board border
    sf::RectangleShape border;
    border.setSize(sf::Vector2f(static_cast<float>(Board::Width * CellSize),
                                static_cast<float>((Board::Height - 1) * CellSize)));
    border.setPosition({boardX, boardY});
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(2.f);
    window.draw(border);
}

void GameView::drawCurrentPiece(sf::RenderWindow& window, const GameState& state,
                                 float offsetX, float offsetY) {
    const Tetromino& piece = state.currentPiece();
    const int baseX = state.pieceX();
    const int baseY = state.pieceY();

    float boardX = BoardOffsetX + offsetX;
    float boardY = BoardOffsetY + offsetY;

    // Draw Ghost Piece
    int ghostY = state.getGhostY();
    sf::Color ghostColor = sf::Color::White;
    ghostColor.a = 64; // Semi-transparent

    for (const auto& offset : piece.getBlocks()) {
        const int x = baseX + offset.x;
        const int y = ghostY + offset.y;
        if (x >= 0 && x < Board::Width && y >= 1 && y < Board::Height) {
            sf::Sprite ghostSprite(m_textureManager->getBlockTexture(piece.getColorId()));
            ghostSprite.setColor(ghostColor);
            ghostSprite.setScale({static_cast<float>(CellSize - 1) / 32.0f,
                                static_cast<float>(CellSize - 1) / 32.0f});
            ghostSprite.setPosition({boardX + static_cast<float>(x * CellSize),
                                    boardY + static_cast<float>((y - 1) * CellSize)});
            window.draw(ghostSprite);
        }
    }

    // Draw actual piece with texture and outline
    sf::Sprite blockSprite(m_textureManager->getBlockTexture(piece.getColorId()));
    blockSprite.setScale({static_cast<float>(CellSize - 1) / 32.0f,
                        static_cast<float>(CellSize - 1) / 32.0f});

    // Draw outline around each block
    sf::RectangleShape outline;
    outline.setSize(sf::Vector2f(static_cast<float>(CellSize - 1),
                                 static_cast<float>(CellSize - 1)));
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineThickness(1.0f);
    outline.setOutlineColor(sf::Color(255, 255, 255, 128));

    for (const auto& offset : piece.getBlocks()) {
        const int x = baseX + offset.x;
        const int y = baseY + offset.y;

        if (x < 0 || x >= Board::Width || y < 1 || y >= Board::Height) {
            continue;
        }

        blockSprite.setPosition({boardX + static_cast<float>(x * CellSize),
                                boardY + static_cast<float>((y - 1) * CellSize)});
        outline.setPosition({boardX + static_cast<float>(x * CellSize),
                            boardY + static_cast<float>((y - 1) * CellSize)});
        window.draw(blockSprite);
        window.draw(outline);
    }
}

void GameView::drawNextPiece(sf::RenderWindow& window, const Tetromino& nextPiece,
                             float offsetX, float offsetY) {
    float previewX = BoardOffsetX + offsetX;
    float previewY = BoardOffsetY + offsetY;

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

    const float pieceOffsetX = previewX + (4 - width) * CellSize / 2.0f;
    const float pieceOffsetY = previewY + (4 - height) * CellSize / 2.0f;

    sf::Sprite blockSprite(m_textureManager->getBlockTexture(nextPiece.getColorId()));
    blockSprite.setScale({static_cast<float>(CellSize - 1) / 32.0f,
                        static_cast<float>(CellSize - 1) / 32.0f});

    for (const auto& b : nextPiece.getBlocks()) {
        const int x = b.x - minX;
        const int y = b.y - minY;
        blockSprite.setPosition({pieceOffsetX + static_cast<float>(x * CellSize),
                                pieceOffsetY + static_cast<float>(y * CellSize)});
        window.draw(blockSprite);
    }
}

void GameView::drawUI(sf::RenderWindow& window, const GameState& state,
                      float offsetX, const std::string& playerLabel) {
    if (!m_fontLoaded) return; // Pas de police, pas d'UI texte

    float uiX;
    if (offsetX == 0.0f && playerLabel.empty()) {
        // Solo mode: position UI to the right of the board
        uiX = BoardOffsetX + Board::Width * CellSize + 50.0f;
    } else {
        // Multiplayer mode: use provided offset
        uiX = BoardOffsetX + offsetX;
    }
    // Both solo and multiplayer UI at the same vertical position
    float uiY = BoardOffsetY + 200.0f;
    const float lineHeight = playerLabel.empty() ? 30.0f : 28.0f;

    // Player label (for multiplayer)
    if (!playerLabel.empty()) {
        sf::Text labelText(m_font, playerLabel);
        labelText.setCharacterSize(28);
        labelText.setFillColor(sf::Color::Yellow);
        labelText.setPosition({uiX, uiY - 40.0f});
        window.draw(labelText);
    }

    // Score
    sf::Text scoreText(m_font, "Score: " + std::to_string(state.score()));
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({uiX, uiY});
    window.draw(scoreText);

    // Mode name and mode-specific info
    uiY += lineHeight;
    const char* modeName = state.getGameMode() ? state.getGameMode()->getModeName() : "Unknown";
    sf::Text modeText(m_font, std::string("Mode: ") + modeName);
    modeText.setCharacterSize(20);
    modeText.setFillColor(sf::Color::Cyan);
    modeText.setPosition({uiX, uiY});
    window.draw(modeText);

    // Show level
    uiY += lineHeight;
    const auto* levelMode = dynamic_cast<const LevelBasedMode*>(state.getGameMode());
    const auto* aiMode = dynamic_cast<const AIMode*>(state.getGameMode());
    if (levelMode) {
        sf::Text levelText(m_font, "Level: " + std::to_string(levelMode->getCurrentLevel()));
        levelText.setCharacterSize(20);
        levelText.setFillColor(sf::Color::Green);
        levelText.setPosition({uiX, uiY});
        window.draw(levelText);
    } else if (aiMode) {
        sf::Text levelText(m_font, "Level: " + std::to_string(aiMode->getCurrentLevel()));
        levelText.setCharacterSize(20);
        levelText.setFillColor(sf::Color::Green);
        levelText.setPosition({uiX, uiY});
        window.draw(levelText);
    }
    
    // Show lines cleared for all modes
    uiY += lineHeight;
    if (state.getGameMode()) {
        sf::Text linesText(m_font, "Lines: " + std::to_string(state.getGameMode()->getLinesCleared()));
        linesText.setCharacterSize(20);
        linesText.setFillColor(sf::Color::Yellow);
        linesText.setPosition({uiX, uiY});
        window.draw(linesText);
    }
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