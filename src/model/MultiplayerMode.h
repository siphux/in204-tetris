#pragma once

#include <memory>
#include <cstdint>

// Forward declarations
class GameState;
class Board;
class Tetromino;

//Multiplayer mode "Marathon" where the winner is the first player to clear a certain number of lines which is set up in the config file
class MarathonGameMode {
public:
    MarathonGameMode(int targetLines = 40);
    
    void update(float deltaTime);
    
    int checkVictory(const GameState& player1, const GameState& player2) const;
    
    int getTargetLines() const { return m_targetLines; }
    int getElapsedTime() const { return m_elapsedTimeMs; }
    
    // Reset
    void reset();
    
private:
    int m_targetLines;
    int m_elapsedTimeMs;
};


class MultiplayerGameMode {
public:
    MultiplayerGameMode(int targetLines = 40);
    
    //update both players
    void update(float deltaTime, GameState& player1, GameState& player2);
    int checkVictory(const GameState& player1, const GameState& player2) const;
    
    const char* getModeName() const;
    int getTargetLines() const;
    int getElapsedTime() const;
    
    void reset();
    
private:
    std::unique_ptr<MarathonGameMode> m_marathonMode;
};
