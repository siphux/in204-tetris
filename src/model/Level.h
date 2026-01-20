#pragma once

// Manages game speed progression.
class Level {
public:
    Level();

    void addLines(int count);

    int current() const;

    // Falling speed depending on level
    float fallSpeed() const;

private:
    int m_level;
    int m_lines;
};
