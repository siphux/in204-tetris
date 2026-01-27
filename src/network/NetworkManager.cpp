#include "NetworkManager.h"
#include <cstring>
#include <iostream>

NetworkManager::NetworkManager() 
    : m_isHost(false), m_isConnected(false) {
}

NetworkManager::~NetworkManager() {
    disconnect();
}

bool NetworkManager::host(unsigned short port) {
    disconnect();
    
    m_isHost = true;
    
    // Set listener to non-blocking so it doesn't freeze the game
    m_listener.setBlocking(false);
    
    if (m_listener.listen(port) != sf::Socket::Status::Done) {
        std::cerr << "Failed to bind to port " << port << std::endl;
        m_isHost = false;
        return false;
    }
    
    std::cout << "Hosting on port " << port << std::endl;
    std::cout << "Waiting for opponent to connect..." << std::endl;
    return true;
}

bool NetworkManager::connect(const std::string& ip, unsigned short port) {
    disconnect();
    
    m_isHost = false;
    
    // Try to connect (blocking for initial connection is okay)
    m_serverSocket.setBlocking(true);
    auto ipAddr = sf::IpAddress::resolve(ip);
    if (!ipAddr.has_value()) {
        std::cerr << "Failed to resolve IP address: " << ip << std::endl;
        return false;
    }
    
    auto status = m_serverSocket.connect(ipAddr.value(), port, sf::seconds(5));
    
    if (status != sf::Socket::Status::Done) {
        std::cerr << "Failed to connect to " << ip << ":" << port << std::endl;
        return false;
    }
    
    // Switch to non-blocking for game loop
    m_serverSocket.setBlocking(false);
    m_isConnected = true;
    
    std::cout << "Connected to " << ip << ":" << port << std::endl;
    return true;
}

void NetworkManager::disconnect() {
    if (m_isHost) {
        m_listener.close();
        m_clientSocket.disconnect();
    } else {
        m_serverSocket.disconnect();
    }
    
    m_isConnected = false;
    m_isHost = false;
}

bool NetworkManager::isConnected() const {
    return m_isConnected;
}

void NetworkManager::update() {
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
        return false;
    }
    
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
    
    auto status = getActiveSocket()->send(packet);
    
    return status == sf::Socket::Status::Done;
}

std::optional<PacketData> NetworkManager::receiveOpponentState() {
    if (!m_isConnected) {
        return std::nullopt;
    }
    
    sf::Packet packet;
    auto status = getActiveSocket()->receive(packet);
    
    // NotReady means no data available (non-blocking)
    if (status == sf::Socket::Status::NotReady) {
        return std::nullopt;
    }
    
    if (status != sf::Socket::Status::Done) {
        // Connection lost
        std::cerr << "Connection lost!" << std::endl;
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
    
    return data;
}

std::string NetworkManager::getLocalIP() {
    auto localAddr = sf::IpAddress::getLocalAddress();
    if (localAddr.has_value()) {
        return localAddr.value().toString();
    }
    return "127.0.0.1";  // Fallback to localhost
}
