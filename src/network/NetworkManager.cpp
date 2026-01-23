#include "NetworkManager.h"
#include "../model/GameState.h"
#include "../model/Board.h"
#include "../model/Tetromino.h"
#include <cstring>

void NetworkManager::serializeBoard(const Board& board, sf::Packet& packet) {
    for (int y = 0; y < Board::Height; y++) {
        for (int x = 0; x < Board::Width; x++) {
            int cellValue = board.getCell(x, y);
            packet << cellValue;
        }
    }
}

void NetworkManager::deserializeBoard(Board& board, sf::Packet& packet) {
    board.clear();
    
    for (int y = 0; y < Board::Height; y++) {
        for (int x = 0; x < Board::Width; x++) {
            int cellValue;
            packet >> cellValue;
            board.setCell(x, y, cellValue);
        }
    }
}

void NetworkManager::serializeTetromino(const Tetromino& tetro, sf::Packet& packet) {
    packet << tetrominoTypeToInt(tetro.getType());
    packet << rotationStateToInt(tetro.getRotationState());
}

void NetworkManager::deserializeTetromino(Tetromino& tetro, sf::Packet& packet) {
    int typeInt;
    packet >> typeInt;
    TetrominoType type = intToTetrominoType(typeInt);
    
    int rotInt;
    packet >> rotInt;
    RotationState rot = intToRotationState(rotInt);
    
    tetro = Tetromino(type);
    tetro.setRotationState(rot);
}

void NetworkManager::serializeGameState(const GameState& state, sf::Packet& packet) {
    serializeBoard(state.board(), packet);
    serializeTetromino(state.currentPiece(), packet);
    packet << state.pieceX() << state.pieceY();
    serializeTetromino(state.nextPiece(), packet);
    packet << state.score();
    packet << state.level();
    int linesCleared = 0;
    if (state.getGameMode()) {
        linesCleared = state.getGameMode()->getLinesCleared();
    }
    packet << linesCleared;
    packet << (state.isGameOver() ? 1 : 0);
}

SerializedGameState NetworkManager::deserializeGameState(sf::Packet& packet) {
    SerializedGameState serialized;
    
    deserializeBoard(serialized.board, packet);
    
    int currentTypeInt, currentRotInt;
    packet >> currentTypeInt >> currentRotInt;
    serialized.currentPieceType = intToTetrominoType(currentTypeInt);
    serialized.currentPieceRotation = intToRotationState(currentRotInt);
    
    packet >> serialized.pieceX >> serialized.pieceY;
    
    int nextTypeInt, nextRotInt;
    packet >> nextTypeInt >> nextRotInt;
    serialized.nextPieceType = intToTetrominoType(nextTypeInt);
    serialized.nextPieceRotation = intToRotationState(nextRotInt);
    
    int gameOverInt;
    packet >> serialized.score >> serialized.level >> serialized.linesCleared >> gameOverInt;
    serialized.gameOver = (gameOverInt == 1);
    
    return serialized;
}

void NetworkManager::applySerializedState(GameState& state, const SerializedGameState& serialized) {
    state.syncBoard(serialized.board);
}

int NetworkManager::tetrominoTypeToInt(TetrominoType type) {
    return static_cast<int>(type);
}

TetrominoType NetworkManager::intToTetrominoType(int value) {
    return static_cast<TetrominoType>(value);
}

int NetworkManager::rotationStateToInt(RotationState state) {
    return static_cast<int>(state);
}

RotationState NetworkManager::intToRotationState(int value) {
    return static_cast<RotationState>(value);
}
