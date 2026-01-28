#pragma once

// Forward declaration
class GameState;

//Abstract base class for all the different game modes implemented
class GameMode {
public:
    virtual ~GameMode() = default;
    virtual void update(float deltaTime, GameState& gameState) = 0;
    virtual float getFallSpeed() const = 0;
    virtual void onLinesClear(int linesCleared, GameState& gameState) = 0;
    virtual void reset() = 0;
    virtual const char* getModeName() const = 0;
    virtual int getLinesCleared() const = 0;
};
