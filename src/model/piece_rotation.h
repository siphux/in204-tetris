#pragma once
#include <cstdint>

class PieceRotation {
public:
    uint16_t mask;  // 4x4
    PieceRotation(uint16_t m);
};