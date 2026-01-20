#include "Score.h"

Score::Score() : m_score(0) {}

void Score::reset() {
    m_score = 0;
}

void Score::addLineClear(int linesCleared, int level) {
    int baseScore = 0;

    switch (linesCleared) {
        case 1: baseScore = 100; break; // Single
        case 2: baseScore = 300; break; // Double
        case 3: baseScore = 500; break; // Triple
        case 4: baseScore = 800; break; // Tetris
        default: return; // No lines or invalid
    }

    m_score += baseScore * (level + 1);
}

int Score::value() const {
    return m_score;
}
