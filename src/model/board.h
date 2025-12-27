#pragma once
#include <cstdint>

#define WIDTH 10
#define HEIGHT 21

class Board {
    public:
        int grid[HEIGHT][WIDTH];

        Board(){
            clear();
        };

        void clear(){
            for (int i = 0; i < HEIGHT; i++){
                 for (int j = 0; j < WIDTH; j++){
                    grid[i][j] = 0;
                 }
            }
        }

        bool isCollision(const Tetromino& piece) const{
            for (const auto& block : piece.getBlocks()){
                int x = piece.getPosition().x + block.x;
                int y = piece.getPosition().y + block.y;

                if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
                    return true;

                if (grid[y][x] != 0)
                    return true;
            }
            return false;
        }

        //à voir comment faire pour éviter que la pièce soit partiellement hors de l'écran
        void lockPiece(const Tetromino& piece){
            for (const auto& block: piece.getBlocks()){
                int x = piece.getPosition().x + block.x;
                int y = piece.getPosition().y + block.y;
                if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT){
                    grid[y][x] = piece.getColorId();
                }
            }
        }

        void fillLines(int lineY){
            for (int y = lineY; y > 0; y--){
                for (int x = 0; x < WIDTH; x++){
                    grid[y][x] = grid[y-1][x];
                }
            }
            for (int x = 0; x < WIDTH; x++) grid[0][x] = 0;
        }

        int clearlines(){
            int linesCleared = 0;
            for (int y = HEIGHT - 1; y >= 0; y--){
                bool isFull = true;
                for (int x = 0; x < WIDTH; x++){
                    if (grid[y][x] == 0){
                        isFull = false;
                        break;
                    }
                }
                if (isFull){
                    linesCleared++;
                    fillLines(y);
                    y++;
                }
            }
            return linesCleared;
        }

};