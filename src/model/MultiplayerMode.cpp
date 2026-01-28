#include "MultiplayerMode.h"
#include "GameState.h"
#include <cstring>

MarathonGameMode::MarathonGameMode(int targetLines)
    : m_targetLines(targetLines), m_elapsedTimeMs(0) {}

void MarathonGameMode::update(float deltaTime) {
    m_elapsedTimeMs += static_cast<int>(deltaTime * 1000.0f);
}

int MarathonGameMode::checkVictory(const GameState& player1, const GameState& player2) const {
    int player1Lines = player1.getGameMode() ? player1.getGameMode()->getLinesCleared() : 0;
    int player2Lines = player2.getGameMode() ? player2.getGameMode()->getLinesCleared() : 0;
    if (player1Lines >= m_targetLines) {
        return 0;
    }
    if (player2Lines >= m_targetLines) {
        return 1;
    }
    
    return -1;
}

void MarathonGameMode::reset() {
    m_elapsedTimeMs = 0;
}

MultiplayerGameMode::MultiplayerGameMode(int targetLines) {
    m_marathonMode = std::make_unique<MarathonGameMode>(targetLines);
}

void MultiplayerGameMode::update(float deltaTime, GameState& player1, GameState& player2) {
    if (m_marathonMode) {
        m_marathonMode->update(deltaTime);
    }
}

int MultiplayerGameMode::checkVictory(const GameState& player1, const GameState& player2) const {
    if (m_marathonMode) {
        return m_marathonMode->checkVictory(player1, player2);
    }
    return -1;
}

const char* MultiplayerGameMode::getModeName() const {
    return "Marathon";
}

int MultiplayerGameMode::getTargetLines() const {
    if (m_marathonMode) {
        return m_marathonMode->getTargetLines();
    }
    return 40;
}

int MultiplayerGameMode::getElapsedTime() const {
    if (m_marathonMode) {
        return m_marathonMode->getElapsedTime();
    }
    return 0;
}

void MultiplayerGameMode::reset() {
    if (m_marathonMode) {
        m_marathonMode->reset();
    }
}
