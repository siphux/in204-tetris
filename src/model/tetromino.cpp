#include "tetromino.h"
#include <stdexcept>

Tetromino::Tetromino(const std::vector<PieceRotation>& r){
    rotations = r;
}

const PieceRotation& Tetromino::getRotation(int index) const {
    if (index < 0 || index >= (int)rotations.size()) {
        throw std::out_of_range("Invalid rotation index");
    }
    return rotations[index];
}

int Tetromino::rotationCount() const {
    return rotations.size();
}