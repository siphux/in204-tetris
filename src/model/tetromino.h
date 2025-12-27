#pragma once
#include <vector>
#include "piece_rotation.h"

class Tetromino {
public:
    Tetromino(const std::vector<PieceRotation>& r);

    const PieceRotation& getRotation(int index) const;
    int rotationCount() const;

private:
    std::vector<PieceRotation> rotations;
};
