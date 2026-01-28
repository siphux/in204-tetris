#pragma once
#include "GameMode.h"
#include "Level.h"
#include "../ai/AIPlayer.h"
#include <memory>

//AI mode where an AI plays
class AIMode : public GameMode {
public:
    AIMode(bool useAdvanced = true);

    void update(float deltaTime, GameState& gameState) override;
    float getFallSpeed() const override;
    void onLinesClear(int linesCleared, GameState& gameState) override;
    void reset() override;
    const char* getModeName() const override;

    // Accessors
    int getLinesCleared() const override;
    int getCurrentLevel() const;

private:
    std::unique_ptr<AIPlayer> m_ai;
    bool m_useAdvanced;  // choose between the two types of AI
    float m_moveTimer;
    Level m_level;
    int m_totalLinesCleared;
    
    static constexpr float BASE_SPEED = 0.5f;
    static constexpr float SPEED_MULTIPLIER = 0.05f;  
};
