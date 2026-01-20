#include "Level.h"

Level::Level() : m_level(0), m_lines(0) {}

void Level::addLines(int count) {
    m_lines += count;
    if (m_lines >= 10) {
        m_lines = 0;
        m_level++;
    }
}

int Level::current() const {
    return m_level;
}

float Level::fallSpeed() const {
    // Faster falling at higher levels
    return 0.5f - 0.05f * m_level;
}
