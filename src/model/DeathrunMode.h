#pragma once
#include "GameMode.h"

// Deathrun mode: fall speed increases continuously over time
// Game accelerates until pieces fall so fast the player can't keep up
class DeathrunMode : public GameMode {
public:
    DeathrunMode();

    void update(float deltaTime, GameState& gameState) override;
    float getFallSpeed() const override;
    void onLinesClear(int linesCleared, GameState& gameState) override;
    void reset() override;
    const char* getModeName() const override;

    // Accessors
    float getElapsedTime() const;
    int getLinesCleared() const override;

private:
    float m_elapsedTime;
    int m_linesCleared;
    
    static constexpr float INITIAL_SPEED = 0.5f;
    static constexpr float ACCELERATION = 0.05f; // increase per second
    static constexpr float MIN_SPEED = 0.05f;
};
