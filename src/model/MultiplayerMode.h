#pragma once

#include <memory>
#include <array>
#include <vector>
#include <cstdint>

// Forward declarations
class GameState;
class Board;
class Tetromino;

// Malus types for attack mode
enum class MalusType {
    GRAVITY_UP,       // Pieces fall faster
    GRAVITY_DOWN,     // Pieces fall slower
    FROZEN,           // Board frozen for duration
    RANDOM_PIECES,    // Next piece is random
    GARBAGE_LINES,    // Add random garbage lines
    INVERT_CONTROLS   // Invert left/right
};

// Malus effect data
struct Malus {
    MalusType type;
    uint32_t durationMs;
    uint8_t intensity;  // 0-255
};

// ============ GAME MODE 1: RACE MODE ============
// Objective: First to clear a target number of lines in the fastest time
// Optional: Can enable to view opponent's board while playing
class RaceGameMode {
public:
    RaceGameMode(int targetLines = 40, bool allowScreenShare = false);
    
    // Update race logic
    void update(float deltaTime);
    
    // Check win condition (returns -1 if no winner, 0/1 for player id)
    int checkVictory(const GameState& player1, const GameState& player2) const;
    
    // Get race statistics
    int getTargetLines() const { return m_targetLines; }
    int getElapsedTime() const { return m_elapsedTimeMs; }
    bool isScreenShareEnabled() const { return m_allowScreenShare; }
    void setScreenShareEnabled(bool enabled) { m_allowScreenShare = enabled; }
    
    // Reset race
    void reset();
    
private:
    int m_targetLines;
    int m_elapsedTimeMs;
    bool m_allowScreenShare;
};

// ============ GAME MODE 2: MALUS MODE ============
// Objective: Knock out opponent while managing maluses
// Features:
//   - Mandatory opponent screen view
//   - Can send maluses/attacks when lines are cleared
//   - Multiple malus types: gravity changes, random pieces, garbage lines, controls invert, etc.
class MalusGameMode {
public:
    MalusGameMode();
    
    // Update malus logic
    void update(float deltaTime);
    
    // Check win condition
    int checkVictory(const GameState& player1, const GameState& player2) const;
    
    // Apply malus to opponent
    void applyMalus(int targetPlayerId, const Malus& malus);
    
    // Get active maluses for a player
    const std::vector<Malus>& getActiveMaluses(int playerId) const;
    
    // Opponent screen view is mandatory
    bool isScreenShareEnabled() const { return true; }
    
    // Reset mode
    void reset();
    
private:
    std::array<std::vector<Malus>, 2> m_playerMaluses;
    int m_elapsedTimeMs;
};

// ============ MULTIPLAYER GAME STATE ============
// Manages the state of both players in a multiplayer game
class MultiplayerGameMode {
public:
    enum class Mode { RACE, MALUS };
    
    MultiplayerGameMode(Mode mode, int targetLines = 40, bool screenShare = false);
    
    // Update both players
    void update(float deltaTime, GameState& player1, GameState& player2);
    
    // Check victory
    int checkVictory(const GameState& player1, const GameState& player2) const;
    
    // Get mode info
    Mode getMode() const { return m_mode; }
    bool isScreenShareEnabled() const;
    int getTargetLines() const;
    
    // Access underlying mode objects
    RaceGameMode* getRaceMode() { return m_raceMode.get(); }
    MalusGameMode* getMalusMode() { return m_malusMode.get(); }
    
    void reset();
    
private:
    Mode m_mode;
    std::unique_ptr<RaceGameMode> m_raceMode;
    std::unique_ptr<MalusGameMode> m_malusMode;
};
