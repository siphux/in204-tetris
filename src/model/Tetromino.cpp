#include "Tetromino.h"

Tetromino::Tetromino(TetrominoType type)
    : m_type(type) {

    // Initialize shape and color depending on type
    switch (type) {
        case TetrominoType::I:
            m_colorId = 1;
            m_blocks = {{-1,0}, {0,0}, {1,0}, {2,0}};
            break;
        case TetrominoType::J:
            m_colorId = 2;
            m_blocks = {{-1,-1}, {-1,0}, {0,0}, {1,0}};
            break;
        case TetrominoType::L:
            m_colorId = 3;
            m_blocks = {{1,-1}, {-1,0}, {0,0}, {1,0}};
            break;
        case TetrominoType::O:
            m_colorId = 4;
            m_blocks = {{0,0}, {1,0}, {0,1}, {1,1}};
            break;
        case TetrominoType::S:
            m_colorId = 5;
            m_blocks = {{0,0}, {1,0}, {-1,1}, {0,1}};
            break;
        case TetrominoType::T:
            m_colorId = 6;
            m_blocks = {{-1,0}, {0,0}, {1,0}, {0,1}};
            break;
        case TetrominoType::Z:
            m_colorId = 7;
            m_blocks = {{-1,0}, {0,0}, {0,1}, {1,1}};
            break;
    }
}

TetrominoType Tetromino::getType() const {
    return m_type;
}

int Tetromino::getColorId() const {
    return m_colorId;
}

const std::vector<Point>& Tetromino::getBlocks() const {
    return m_blocks;
}
