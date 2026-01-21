#include "LevelBasedMode.h"
#include "GameState.h"

LevelBasedMode::LevelBasedMode() : m_level() {}

void LevelBasedMode::update(float deltaTime, GameState& gameState) {
    // Level-based mode doesn't need per-frame updates
    // Level progression is handled in onLinesClear
}

float LevelBasedMode::getFallSpeed() const {
    int level = m_level.current();
    float speed = BASE_SPEED - (SPEED_MULTIPLIER * level);
    // Ensure minimum speed
    return speed > 0.05f ? speed : 0.05f;
}

void LevelBasedMode::onLinesClear(int linesCleared, GameState& gameState) {
    // Add cleared lines to level tracker
    m_level.addLines(linesCleared);
}

void LevelBasedMode::reset() {
    m_level = Level();
}

const char* LevelBasedMode::getModeName() const {
    return "Level Mode";
}

int LevelBasedMode::getCurrentLevel() const {
    return m_level.current();
}

int LevelBasedMode::getLinesCleared() const {
    // This would require extending Level class, but we can calculate from level
    return m_level.current() * 10; // Approximation
}
