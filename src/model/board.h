#pragma once
#include <cstdint>

class Board {
    public:
        static constexpr int WIDTH = 10;
        static constexpr int HEIGHT = 21;

        Board();

        uint16_t getRow(int y) const;
        bool canPlace(uint16_t mask, int x, int y) const;
        void place(uint16_t mask, int x, int y);
        int clearFullRows();

    private:
        uint16_t rows[HEIGHT];
};