#pragma once

// Forward declaration
class GameState;

// Base class for different game modes
// Defines the interface that all game modes must implement
class GameMode {
public:
    virtual ~GameMode() = default;

    // Update mode-specific logic (e.g., level progression, speed calculations)
    virtual void update(float deltaTime, GameState& gameState) = 0;

    // Get the fall speed for the current mode
    virtual float getFallSpeed() const = 0;

    // Handle line clears (may trigger different effects per mode)
    virtual void onLinesClear(int linesCleared, GameState& gameState) = 0;

    // Reset mode to initial state
    virtual void reset() = 0;

    // Get display name for UI
    virtual const char* getModeName() const = 0;
    
    // Get total lines cleared in this mode
    virtual int getLinesCleared() const = 0;
};
