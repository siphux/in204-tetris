#pragma once
#include <vector>
#include "piece_rotation.h"

class Point{
    public:
    int x;
    int y;

    Point(int X, int Y): x(X), y(Y) {}
}

enum TetrominoType { I, J, L, O, S, T, Z};


class Tetromino {
    private:
        TetrominoType type;    
        int rotation_state;
        Point board_position;
        std::vector<Point> blocks;
    
    public:
        Tetromino(TetrominoType t){
            type = t;
            rotation_state = 0;
            board_position = Point(4,0);
            switch(t){
                case I: colorId = 1; blocks = {{-1, 0}, {0, 0}, {1, 0}, {2, 0}}; break; // Cyan
                case J: colorId = 2; blocks = {{-1,-1}, {-1,0}, {0, 0}, {1, 0}}; break; // Blue
                case L: colorId = 3; blocks = {{1, -1}, {-1,0}, {0, 0}, {1, 0}}; break; // Orange
                case O: colorId = 4; blocks = {{0,  0}, {1, 0}, {0, 1}, {1, 1}}; break; // Yellow
                case S: colorId = 5; blocks = {{0,  0}, {1, 0}, {-1,1}, {0, 1}}; break; // Green
                case T: colorId = 6; blocks = {{-1, 0}, {0, 0}, {1, 0}, {0, 1}}; break; // Purple
                case Z: colorId = 7; blocks = {{-1, 0}, {0, 0}, {0, 1}, {1, 1}}; break; // Red
            }
        }

        TetrominoType getType() const {
            return type;
        }
        Point getPosition() const {
            return board_position;
        }
        void setPosition(int x, int y) {
            board_position.x = x;
            board_position.y = y;
        }
        std::vector<Point> getBlocks() const {
            return blocks;
        }
        int getRotationState() const {
            return rotation_state;
        }

        void rotateClockwise(){
            rotation_state = (rotation_state + 1)%4;
            for (auto& block : blocks){
                int temp = block.x;
                block.x = -block.y;
                block.y = temp;
            }
        }
};
