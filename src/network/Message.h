#pragma once
#include <SFML/Network.hpp>
#include <cstdint>

enum class MessageType {
    JOIN,           // Client demande de rejoindre
    JOIN_ACK,       // Serveur confirme la connexion (avec playerId)
    START,          // Serveur démarre la partie
    MOVE,           // Client envoie mouvement (LEFT/RIGHT)
    ROTATE,         // Client envoie rotation (CW/CCW)
    DROP,           // Client envoie drop (SOFT/HARD)
    GAME_STATE,     // Serveur envoie état complet
    LINES_CLEARED,  // Client notifie lignes nettoyées
    ATTACK,         // Serveur envoie malus (mode avec malus)
    GAME_OVER,      // Fin de partie (victoire/défaite)
    DISCONNECT      // Déconnexion
};

class Message {
public:
    Message();
    explicit Message(MessageType type, int playerId = -1);

    // Getters
    MessageType getType() const;
    int getPlayerId() const;
    uint32_t getTimestamp() const;

    // Setters
    void setPlayerId(int id);
    void setTimestamp(uint32_t ts);

    // Payload
    sf::Packet& payload();
    const sf::Packet& payload() const;
    void clearPayload();

    // Network
    bool send(sf::TcpSocket& socket) const;
    static bool receive(sf::TcpSocket& socket, Message& outMessage);

private:
    void serialize(sf::Packet& packet) const;
    void deserialize(sf::Packet& packet);
    
    MessageType m_type;
    int m_playerId;
    uint32_t m_timestamp;
    sf::Packet m_payload;
};
