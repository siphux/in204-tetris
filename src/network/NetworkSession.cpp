#include "NetworkSession.h"
#include <cstring>
#include <iostream>
#include <sstream>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <ifaddrs.h>
    #include <arpa/inet.h>
    #include <netdb.h>
#endif

NetworkSession::NetworkSession(SessionRole role, const std::string& playerName)
    : m_role(role),
      m_status(ConnectionStatus::DISCONNECTED),
      m_playerName(playerName),
      m_latencyMs(0) {
}

NetworkSession::~NetworkSession() {
    disconnect();
}

bool NetworkSession::startHosting(unsigned short port) {
    if (m_role != SessionRole::HOST) {
        m_lastError = "Not configured as host";
        return false;
    }

    m_listener = std::make_unique<sf::TcpListener>();
    m_listener->setBlocking(false);

    if (m_listener->listen(port) != sf::Socket::Status::Done) {
        m_lastError = "Failed to bind listening port";
        return false;
    }

    m_status = ConnectionStatus::CONNECTING;
    return true;
}

unsigned short NetworkSession::getListeningPort() const {
    if (m_listener) {
        return m_listener->getLocalPort();
    }
    return 0;
}

std::string NetworkSession::getLocalIP() const {
    return getSystemLocalIP();
}

bool NetworkSession::pollIncomingConnection() {
    if (m_role != SessionRole::HOST || !m_listener) {
        return false;
    }

    m_socket = std::make_unique<sf::TcpSocket>();
    m_socket->setBlocking(false);

    if (m_listener->accept(*m_socket) == sf::Socket::Status::Done) {
        m_status = ConnectionStatus::CONNECTED;
        m_connectionStartTime = std::chrono::steady_clock::now();
        m_lastHeartbeatReceived = m_connectionStartTime;
        return true;
    }

    return false;
}

bool NetworkSession::connectToHost(const std::string& hostAddress, unsigned short port, int timeoutMs) {
    if (m_role != SessionRole::CLIENT) {
        m_lastError = "Not configured as client";
        return false;
    }

    m_socket = std::make_unique<sf::TcpSocket>();
    
    auto ipAddressOpt = sf::IpAddress::resolve(hostAddress);
    if (!ipAddressOpt.has_value()) {
        m_lastError = "Failed to resolve host address: " + hostAddress;
        m_status = ConnectionStatus::ERROR;
        return false;
    }
    sf::IpAddress ipAddress = ipAddressOpt.value();
    
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        sf::Socket::Status status = m_socket->connect(ipAddress, port, sf::seconds(1.0f));
        
        if (status == sf::Socket::Status::Done) {
            m_socket->setBlocking(false);
            m_status = ConnectionStatus::CONNECTED;
            m_connectionStartTime = std::chrono::steady_clock::now();
            m_lastHeartbeatReceived = m_connectionStartTime;
            return true;
        }
        
        if (status == sf::Socket::Status::Error) {
            m_lastError = "Failed to connect to host: " + hostAddress + ":" + std::to_string(port);
            m_status = ConnectionStatus::ERROR;
            return false;
        }
        
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();
        if (elapsed > timeoutMs) {
            m_lastError = "Connection timeout";
            m_status = ConnectionStatus::ERROR;
            return false;
        }
    }
}

bool NetworkSession::sendPacket(const uint8_t* data, size_t size) {
    if (!isConnected() || !m_socket) {
        return false;
    }

    if (m_socket->send(data, size) == sf::Socket::Status::Done) {
        return true;
    }

    return false;
}

size_t NetworkSession::receivePacket(uint8_t* outData, size_t maxSize) {
    if (!isConnected() || !m_socket) {
        return 0;
    }

    size_t received = 0;
    if (m_socket->receive(outData, maxSize, received) == sf::Socket::Status::Done) {
        return received;
    }

    return 0;
}

void NetworkSession::sendHeartbeat() {
    if (!isConnected()) {
        return;
    }

    uint8_t heartbeat = 0xFF;
    sendPacket(&heartbeat, 1);
    m_lastHeartbeatSent = std::chrono::steady_clock::now();
}

void NetworkSession::update() {
    updateConnectionTimeout();
    updateHeartbeat();
}

void NetworkSession::updateConnectionTimeout() {
    if (m_status != ConnectionStatus::CONNECTED) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastHeartbeat = std::chrono::duration_cast<std::chrono::seconds>(
        now - m_lastHeartbeatReceived).count();

    if (timeSinceLastHeartbeat > 10) {
        disconnect();
        m_lastError = "Connection timeout - no heartbeat received";
        m_status = ConnectionStatus::ERROR;
    }
}

void NetworkSession::updateHeartbeat() {
    if (m_status != ConnectionStatus::CONNECTED) {
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastSent = std::chrono::duration_cast<std::chrono::seconds>(
        now - m_lastHeartbeatSent).count();

    if (timeSinceLastSent >= 2) {
        sendHeartbeat();
    }
}

void NetworkSession::disconnect() {
    if (m_socket) {
        m_socket->disconnect();
    }
    m_status = ConnectionStatus::DISCONNECTED;
}

void NetworkSession::setBlocking(bool blocking) {
    if (m_socket) {
        m_socket->setBlocking(blocking);
    }
    if (m_listener) {
        m_listener->setBlocking(blocking);
    }
}

std::string NetworkSession::getSystemLocalIP() const {
#ifdef _WIN32
    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) != SOCKET_ERROR) {
        struct hostent* hostEntry = gethostbyname(hostName);
        if (hostEntry) {
            struct in_addr ipAddr;
            ipAddr.S_un.S_addr = *(u_long*)hostEntry->h_addr_list[0];
            return std::string(inet_ntoa(ipAddr));
        }
    }
    return "127.0.0.1";
#else
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) {
        return "127.0.0.1";
    }

    std::string result = "127.0.0.1";
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            if (std::string(ifa->ifa_name) != "lo") {
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr,
                         ip, INET_ADDRSTRLEN);
                result = std::string(ip);
                break;
            }
        }
    }

    if (ifaddr) {
        freeifaddrs(ifaddr);
    }

    return result;
#endif
}
