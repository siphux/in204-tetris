#pragma once
#include "GameMode.h"
#include "Level.h"

// Level-based mode: speed increases with level progression
// Level advances every N lines cleared
class LevelBasedMode : public GameMode {
public:
    LevelBasedMode();

    void update(float deltaTime, GameState& gameState) override;
    float getFallSpeed() const override;
    void onLinesClear(int linesCleared, GameState& gameState) override;
    void reset() override;
    const char* getModeName() const override;

    // Accessors
    int getCurrentLevel() const;
    int getLinesCleared() const;

private:
    Level m_level;
    
    static constexpr float BASE_SPEED = 0.5f;
    static constexpr float SPEED_MULTIPLIER = 0.05f;
};
