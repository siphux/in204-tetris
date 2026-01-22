#include "Message.h"

Message::Message()
    : m_type(MessageType::JOIN),
      m_playerId(-1),
      m_timestamp(0) {}

Message::Message(MessageType type, int playerId)
    : m_type(type),
      m_playerId(playerId),
      m_timestamp(0) {}

MessageType Message::getType() const {
    return m_type;
}

int Message::getPlayerId() const {
    return m_playerId;
}

uint32_t Message::getTimestamp() const {
    return m_timestamp;
}

void Message::setPlayerId(int id) {
    m_playerId = id;
}

void Message::setTimestamp(uint32_t ts) {
    m_timestamp = ts;
}

sf::Packet& Message::payload() {
    return m_payload;
}

const sf::Packet& Message::payload() const {
    return m_payload;
}

void Message::clearPayload() {
    m_payload.clear();
}

void Message::serialize(sf::Packet& packet) const {
    packet << static_cast<uint8_t>(m_type);
    packet << m_playerId;                   
    packet << m_timestamp;               
    packet.append(m_payload.getData(), m_payload.getDataSize());
}

void Message::deserialize(sf::Packet& packet) {
    uint8_t type;
    packet >> type;
    packet >> m_playerId;
    packet >> m_timestamp;

    m_type = static_cast<MessageType>(type);

    m_payload.clear();

    const void* data = static_cast<const char*>(packet.getData()) + packet.getReadPosition();
    std::size_t size = packet.getDataSize() - packet.getReadPosition();
    m_payload.append(data, size);
}

bool Message::send(sf::TcpSocket& socket) const {
    sf::Packet packet;
    serialize(packet);
    return socket.send(packet) == sf::Socket::Done;
}

bool Message::receive(sf::TcpSocket& socket, Message& outMessage) {
    sf::Packet packet;
    sf::Socket::Status status = socket.receive(packet);

    if (status != sf::Socket::Done) {
        return false;
    }

    outMessage.deserialize(packet);
    return true;
}
