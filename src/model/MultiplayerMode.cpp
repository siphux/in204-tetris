#include "MultiplayerMode.h"
#include "GameState.h"
#include <cstring>

// ============ RACE MODE IMPLEMENTATION ============
RaceGameMode::RaceGameMode(int targetLines, bool allowScreenShare)
    : m_targetLines(targetLines), m_elapsedTimeMs(0), m_allowScreenShare(allowScreenShare) {}

void RaceGameMode::update(float deltaTime) {
    m_elapsedTimeMs += static_cast<int>(deltaTime * 1000.0f);
}

int RaceGameMode::checkVictory(const GameState& player1, const GameState& player2) const {
    // Race: first to reach target lines wins
    int player1Lines = player1.getGameMode() ? player1.getGameMode()->getLinesCleared() : 0;
    int player2Lines = player2.getGameMode() ? player2.getGameMode()->getLinesCleared() : 0;
    
    if (player1Lines >= m_targetLines) {
        return 0;  // Player 1 wins
    }
    if (player2Lines >= m_targetLines) {
        return 1;  // Player 2 wins
    }
    
    return -1;  // No winner yet
}

void RaceGameMode::reset() {
    m_elapsedTimeMs = 0;
}

// ============ MALUS MODE IMPLEMENTATION ============
MalusGameMode::MalusGameMode() : m_elapsedTimeMs(0) {
    m_playerMaluses[0].clear();
    m_playerMaluses[1].clear();
}

void MalusGameMode::update(float deltaTime) {
    m_elapsedTimeMs += static_cast<int>(deltaTime * 1000.0f);
    
    // Update malus durations
    for (int player = 0; player < 2; ++player) {
        auto& maluses = m_playerMaluses[player];
        for (auto it = maluses.begin(); it != maluses.end(); ) {
            it->durationMs -= static_cast<int>(deltaTime * 1000.0f);
            if (it->durationMs <= 0) {
                it = maluses.erase(it);
            } else {
                ++it;
            }
        }
    }
}

int MalusGameMode::checkVictory(const GameState& player1, const GameState& player2) const {
    // MALUS: last player standing (not game over = wins)
    bool p1GameOver = player1.isGameOver();
    bool p2GameOver = player2.isGameOver();
    
    // If only one is game over, the other wins
    if (p1GameOver && !p2GameOver) return 1;  // Player 2 wins
    if (!p1GameOver && p2GameOver) return 0;  // Player 1 wins
    if (p1GameOver && p2GameOver) return 2;   // Tie (both died)
    
    return -1;  // Game still running
}

void MalusGameMode::applyMalus(int targetPlayerId, const Malus& malus) {
    if (targetPlayerId >= 0 && targetPlayerId < 2) {
        m_playerMaluses[targetPlayerId].push_back(malus);
    }
}

const std::vector<Malus>& MalusGameMode::getActiveMaluses(int playerId) const {
    if (playerId >= 0 && playerId < 2) {
        return m_playerMaluses[playerId];
    }
    static const std::vector<Malus> empty;
    return empty;
}

void MalusGameMode::reset() {
    m_elapsedTimeMs = 0;
    m_playerMaluses[0].clear();
    m_playerMaluses[1].clear();
}

// ============ MULTIPLAYER GAME MODE IMPLEMENTATION ============
MultiplayerGameMode::MultiplayerGameMode(Mode mode, int targetLines, bool screenShare)
    : m_mode(mode) {
    if (mode == Mode::RACE) {
        m_raceMode = std::make_unique<RaceGameMode>(targetLines, screenShare);
    } else {
        m_malusMode = std::make_unique<MalusGameMode>();
    }
}

void MultiplayerGameMode::update(float deltaTime, GameState& player1, GameState& player2) {
    if (m_mode == Mode::RACE) {
        if (m_raceMode) m_raceMode->update(deltaTime);
    } else {
        if (m_malusMode) m_malusMode->update(deltaTime);
    }
}

int MultiplayerGameMode::checkVictory(const GameState& player1, const GameState& player2) const {
    if (m_mode == Mode::RACE) {
        if (m_raceMode) return m_raceMode->checkVictory(player1, player2);
    } else {
        if (m_malusMode) return m_malusMode->checkVictory(player1, player2);
    }
    return -1;
}

bool MultiplayerGameMode::isScreenShareEnabled() const {
    if (m_mode == Mode::RACE) {
        return m_raceMode ? m_raceMode->isScreenShareEnabled() : false;
    } else {
        return true;  // Mandatory for malus mode
    }
}

int MultiplayerGameMode::getTargetLines() const {
    if (m_mode == Mode::RACE) {
        return m_raceMode ? m_raceMode->getTargetLines() : 40;
    }
    return 0;
}

void MultiplayerGameMode::reset() {
    if (m_raceMode) m_raceMode->reset();
    if (m_malusMode) m_malusMode->reset();
}
