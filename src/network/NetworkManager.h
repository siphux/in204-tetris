#pragma once
#include <SFML/Network.hpp>
#include <optional>
#include <cstdint>

// Simplified game state for network transmission (880 bytes fixed size)
struct PacketData {
    int32_t grid[21][10];  // 21 rows x 10 columns (includes hidden spawn row)
    int32_t currentPieceType;
    int32_t currentPieceX;
    int32_t currentPieceY;
    int32_t currentPieceRotation;
    int32_t score;
    int32_t level;
    bool isGameOver;
    bool isReady;  // Player ready status
    
    PacketData() {
        for (int y = 0; y < 21; y++) {
            for (int x = 0; x < 10; x++) {
                grid[y][x] = 0;
            }
        }
        currentPieceType = 0;
        currentPieceX = 0;
        currentPieceY = 0;
        currentPieceRotation = 0;
        score = 0;
        level = 1;
        isGameOver = false;
        isReady = false;
    }
};

// NetworkManager handles LAN multiplayer for Tetris
// Uses non-blocking TCP sockets for real-time game state sync
class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    
    // Host a game (act as server)
    bool host(unsigned short port = 53000);
    
    // Connect to a host (act as client)
    bool connect(const std::string& ip, unsigned short port = 53000);
    
    // Disconnect and cleanup
    void disconnect();
    
    // Check if we're connected (as host or client)
    bool isConnected() const;
    
    // Check if we're the host
    bool isHost() const { return m_isHost; }
    
    // Send our game state to opponent
    bool sendGameState(const PacketData& data);
    
    // Receive opponent's game state (non-blocking)
    std::optional<PacketData> receiveOpponentState();
    
    // Update networking (call every frame) - handles accepting connections
    void update();
    
    // Get local IP for LAN play
    static std::string getLocalIP();
    
private:
    bool m_isHost;
    bool m_isConnected;
    
    // For host: listener + client socket
    sf::TcpListener m_listener;
    sf::TcpSocket m_clientSocket;  // The connected peer
    
    // For client: just the connection to host
    sf::TcpSocket m_serverSocket;  // Connection to host
    
    // Helper: get the active socket (host uses m_clientSocket, client uses m_serverSocket)
    sf::TcpSocket* getActiveSocket();
};
