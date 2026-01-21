#pragma once
#include "GameMode.h"
#include "../ai/SimpleAI.h"
#include <memory>

// AI Mode: An AI player automatically plays the game
// The AI uses heuristics to make optimal piece placements
class AIMode : public GameMode {
public:
    AIMode();

    void update(float deltaTime, GameState& gameState) override;
    float getFallSpeed() const override;
    void onLinesClear(int linesCleared, GameState& gameState) override;
    void reset() override;
    const char* getModeName() const override;

    // Accessors
    int getLinesCleared() const override;

private:
    std::unique_ptr<SimpleAI> m_ai;
    float m_moveTimer;
    int m_linesCleared;
    
    static constexpr float BASE_SPEED = 0.3f;  // Faster than normal for AI
    static constexpr float MOVE_DELAY = 0.2f; // Time between AI moves in seconds
};
