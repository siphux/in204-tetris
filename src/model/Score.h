#pragma once

// Handles scoring following a TETRIS 99–like system
class Score {
public:
    Score();

    // Add score depending on cleared lines and current level
    void addLineClear(int linesCleared, int level);

    // Direct access to current score
    int value() const;

    // Reset score (new game)
    void reset();

private:
    int m_score;
};
