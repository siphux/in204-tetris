#include "DeathrunMode.h"
#include "GameState.h"

DeathrunMode::DeathrunMode() : m_elapsedTime(0.0f) {}

void DeathrunMode::update(float deltaTime, GameState& gameState) {
    m_elapsedTime += deltaTime;
}

float DeathrunMode::getFallSpeed() const {
    // Speed = initial - (acceleration * elapsed time)
    // Acceleration makes speed smaller (faster fall)
    float speed = INITIAL_SPEED - (ACCELERATION * m_elapsedTime);
    return speed > MIN_SPEED ? speed : MIN_SPEED;
}

void DeathrunMode::onLinesClear(int linesCleared, GameState& gameState) {
    // Deathrun mode doesn't have special line clear mechanics
    // Just keep accelerating based on time
}

void DeathrunMode::reset() {
    m_elapsedTime = 0.0f;
}

const char* DeathrunMode::getModeName() const {
    return "Deathrun Mode";
}

float DeathrunMode::getElapsedTime() const {
    return m_elapsedTime;
}
