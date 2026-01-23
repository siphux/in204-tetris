#pragma once
#include <SFML/Network.hpp>
#include <cstdint>
#include <vector>

enum class MessageType {
    JOIN,
    JOIN_ACK,
    START,
    MOVE,
    ROTATE,
    DROP,
    GAME_STATE,
    LINES_CLEARED,
    ATTACK,
    GAME_OVER,
    DISCONNECT
};

class Message {
public:
    Message();
    explicit Message(MessageType type, int playerId = -1);

    MessageType getType() const;
    int getPlayerId() const;
    uint32_t getTimestamp() const;

    void setPlayerId(int id);
    void setTimestamp(uint32_t ts);

    sf::Packet& payload();
    const sf::Packet& payload() const;
    void clearPayload();

    bool send(sf::TcpSocket& socket) const;
    static bool receive(sf::TcpSocket& socket, Message& outMessage);
    
    std::vector<uint8_t> serialize() const;
    
    static Message deserialize(const uint8_t* data, size_t size);

private:
    void serialize(sf::Packet& packet) const;
    void deserialize(sf::Packet& packet);
    
    MessageType m_type;
    int m_playerId;
    uint32_t m_timestamp;
    sf::Packet m_payload;
};
