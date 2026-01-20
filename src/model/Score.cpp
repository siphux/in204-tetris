#include "Score.h"

Score::Score() : m_score(0) {}

void Score::reset() {
    m_score = 0;
}

void Score::addLineClear(int linesCleared, int level) {
    int baseScore = 0;

    switch (linesCleared) {
        case 1: baseScore = 40; break;   // Single
        case 2: baseScore = 100; break;  // Double
        case 3: baseScore = 300; break;  // Triple
        case 4: baseScore = 1200; break; // Tetris
        default: return;
    }

    // Formule: f(p, n) = p * (n + 1)
    // où p = baseScore, n = level
    m_score += baseScore * (level + 1);
}

int Score::value() const {
    return m_score;
}
