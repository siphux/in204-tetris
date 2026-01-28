#include "NetworkManager.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

NetworkManager::NetworkManager() 
    : m_isHost(false), m_isConnected(false) {
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::host(unsigned short port) {
    try {
        disconnect();
        
        m_isHost = true;
        
        // Set listener to non-blocking so it doesn't freeze the game
        m_listener.setBlocking(false);
        
        if (m_listener.listen(port) != sf::Socket::Status::Done) {
            std::cerr << "Error: Failed to bind to port " << port << std::endl;
            m_isHost = false;
            return false;
        }
        
        std::string localIP = getLocalIP();
        std::cout << "\n=== SERVER STARTED ===" << std::endl;
        std::cout << "IP Address: " << localIP << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Share this with your friend: " << localIP << ":" << port << std::endl;
        std::cout << "Waiting for opponent to connect..." << std::endl;
        std::cout << "==================\n" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while hosting: " << e.what() << std::endl;
        m_isHost = false;
        return false;
    }
}

bool NetworkManager::connect(const std::string& ip, unsigned short port) {
    try {
        disconnect();
        
        m_isHost = false;
        
        // Try to connect (blocking for initial connection is okay)
        m_serverSocket.setBlocking(true);
        auto ipAddr = sf::IpAddress::resolve(ip);
        if (!ipAddr.has_value()) {
            std::cerr << "Error: Failed to resolve IP address: " << ip << std::endl;
            return false;
        }
        
        auto status = m_serverSocket.connect(ipAddr.value(), port, sf::seconds(5));
        
        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error: Failed to connect to " << ip << ":" << port << std::endl;
            return false;
        }
        
        // Switch to non-blocking for game loop
        m_serverSocket.setBlocking(false);
        m_isConnected = true;
        
        std::cout << "Connected to " << ip << ":" << port << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while connecting: " << e.what() << std::endl;
        return false;
    }
}

void NetworkManager::disconnect() {
    try {
        if (m_isHost) {
            m_listener.close();
            m_clientSocket.disconnect();
        } else {
            m_serverSocket.disconnect();
        }
        
        m_isConnected = false;
        m_isHost = false;
    } catch (const std::exception& e) {
        std::cerr << "Error during disconnect: " << e.what() << std::endl;
        m_isConnected = false;
        m_isHost = false;
    }
}

bool NetworkManager::isConnected() const {
    return m_isConnected;
}

void NetworkManager::update() {
    try {
        // If we're hosting and not yet connected, try to accept a connection
        if (m_isHost && !m_isConnected) {
            auto status = m_listener.accept(m_clientSocket);
            
            if (status == sf::Socket::Status::Done) {
                m_clientSocket.setBlocking(false);
                m_isConnected = true;
                std::cout << "Client connected!" << std::endl;
            }
            // Status::NotReady means no connection yet (non-blocking)
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during network update: " << e.what() << std::endl;
    }
}

sf::TcpSocket* NetworkManager::getActiveSocket() {
    if (m_isHost) {
        return &m_clientSocket;
    } else {
        return &m_serverSocket;
    }
}

bool NetworkManager::sendGameState(const PacketData& data) {
    if (!m_isConnected) {
        std::cerr << "Error: Not connected to send game state" << std::endl;
        return false;
    }
    
    try {
        sf::Packet packet;
        
        // Serialize the PacketData
        for (int y = 0; y < 21; y++) {
            for (int x = 0; x < 10; x++) {
                packet << data.grid[y][x];
            }
        }
        
        packet << data.currentPieceType;
        packet << data.currentPieceX;
        packet << data.currentPieceY;
        packet << data.currentPieceRotation;
        packet << data.score;
        packet << data.level;
        packet << data.isGameOver;
        packet << data.isReady;
        
        auto status = getActiveSocket()->send(packet);
        
        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error: Failed to send game state (status: " << static_cast<int>(status) << ")" << std::endl;
            if (status == sf::Socket::Status::Disconnected) {
                m_isConnected = false;
            }
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception while sending game state: " << e.what() << std::endl;
        return false;
    }
}

std::optional<PacketData> NetworkManager::receiveOpponentState() {
    if (!m_isConnected) {
        std::cerr << "Error: Not connected to receive game state" << std::endl;
        return std::nullopt;
    }
    
    try {
        sf::Packet packet;
        auto status = getActiveSocket()->receive(packet);
        
        // NotReady means no data available (non-blocking)
        if (status == sf::Socket::Status::NotReady) {
            return std::nullopt;
        }
        
        if (status != sf::Socket::Status::Done) {
            // Connection lost or error
            std::cerr << "Error: Connection lost or receive failed (status: " << static_cast<int>(status) << ")" << std::endl;
            m_isConnected = false;
            return std::nullopt;
        }
        
        // Deserialize
        PacketData data;
        
        for (int y = 0; y < 21; y++) {
            for (int x = 0; x < 10; x++) {
                packet >> data.grid[y][x];
            }
        }
        
        packet >> data.currentPieceType;
        packet >> data.currentPieceX;
        packet >> data.currentPieceY;
        packet >> data.currentPieceRotation;
        packet >> data.score;
        packet >> data.level;
        packet >> data.isGameOver;
        packet >> data.isReady;
        
        return data;
    } catch (const std::exception& e) {
        std::cerr << "Exception while receiving game state: " << e.what() << std::endl;
        m_isConnected = false;
        return std::nullopt;
    }
}

std::string NetworkManager::getLocalIP() {
    auto localAddr = sf::IpAddress::getLocalAddress();
    if (localAddr.has_value()) {
        return localAddr.value().toString();
    }
    return "127.0.0.1";  // Fallback to localhost
}
