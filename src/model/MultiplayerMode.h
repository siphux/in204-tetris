#pragma once

#include <memory>
#include <cstdint>

// Forward declarations
class GameState;
class Board;
class Tetromino;

// ============ MARATHON MODE ============
// Objective: First to clear a target number of lines wins
class MarathonGameMode {
public:
    MarathonGameMode(int targetLines = 40);
    
    // Update logic
    void update(float deltaTime);
    
    // Check win condition (returns -1 if no winner, 0/1 for player id)
    int checkVictory(const GameState& player1, const GameState& player2) const;
    
    // Get statistics
    int getTargetLines() const { return m_targetLines; }
    int getElapsedTime() const { return m_elapsedTimeMs; }
    
    // Reset
    void reset();
    
private:
    int m_targetLines;
    int m_elapsedTimeMs;
};

// ============ MULTIPLAYER GAME STATE ============
// Manages the state of both players in a multiplayer game
class MultiplayerGameMode {
public:
    MultiplayerGameMode(int targetLines = 40);
    
    // Update both players
    void update(float deltaTime, GameState& player1, GameState& player2);
    
    // Check victory
    int checkVictory(const GameState& player1, const GameState& player2) const;
    
    // Get mode info
    int getTargetLines() const;
    
    // Access underlying mode
    MarathonGameMode* getMarathonMode() { return m_marathonMode.get(); }
    
    void reset();
    
private:
    std::unique_ptr<MarathonGameMode> m_marathonMode;
};
