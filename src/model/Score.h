#pragma once

class Score {
public:
    Score();

    // Add score according to the number of lines cleared with the move and the level
    void addLineClear(int linesCleared, int level);

    // Access current score
    int value() const;

    // Reset score (for new game)
    void reset();

private:
    int m_score;
};
