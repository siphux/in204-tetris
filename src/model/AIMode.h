#pragma once
#include "GameMode.h"
#include "Level.h"
#include "../ai/AIPlayer.h"
#include <memory>

// AI Mode: An AI player automatically plays the game
// The AI uses heuristics to make optimal piece placements
// Uses level-based progression similar to Level Mode
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
    bool m_useAdvanced;  // Store the AI type choice
    float m_moveTimer;
    Level m_level;
    int m_totalLinesCleared;
    
    static constexpr float BASE_SPEED = 0.5f;  // Base speed for AI
    static constexpr float SPEED_MULTIPLIER = 0.05f;  // Speed increase per level
    static constexpr float MOVE_DELAY = 0.2f; // Time between AI moves in seconds
};
