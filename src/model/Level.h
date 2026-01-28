#pragma once

//Levels management for the level based game mode
class Level {
public:
    Level();

     void addLines(int count);
    int current() const;
    float fallSpeed() const;

private:
    int m_level;
    int m_lines;
};
