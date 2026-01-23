#pragma once
#include <SFML/Network.hpp>
#include "../model/Board.h"
#include "../model/GameState.h"
#include "../model/Tetromino.h"

struct SerializedGameState {
    Board board;
    TetrominoType currentPieceType;
    RotationState currentPieceRotation;
    int pieceX;
    int pieceY;
    TetrominoType nextPieceType;
    RotationState nextPieceRotation;
    int score;
    int level;
    int linesCleared;
    bool gameOver;
};

class NetworkManager {
public:
    static void serializeBoard(const Board& board, sf::Packet& packet);
    static void deserializeBoard(Board& board, sf::Packet& packet);
    
    static void serializeTetromino(const Tetromino& tetro, sf::Packet& packet);
    static void deserializeTetromino(Tetromino& tetro, sf::Packet& packet);
    
    static void serializeGameState(const GameState& state, sf::Packet& packet);
    static SerializedGameState deserializeGameState(sf::Packet& packet);
    
    static void applySerializedState(GameState& state, const SerializedGameState& serialized);
    
    static int tetrominoTypeToInt(TetrominoType type);
    static TetrominoType intToTetrominoType(int value);
    static int rotationStateToInt(RotationState state);
    static RotationState intToRotationState(int value);
};
