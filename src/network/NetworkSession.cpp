#include "NetworkSession.h"
#include "NetworkConfig.h"
#include <SFML/Network/IpAddress.hpp>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

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
      m_latencyMs(0),
      m_reconnectPort(0),
      m_reconnectAttempts(0) {
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

    // Bind to 0.0.0.0 (all interfaces) so it works in WSL2
    // This allows connections from both WSL2 network and Windows host network
    if (m_listener->listen(port, sf::IpAddress::Any) != sf::Socket::Status::Done) {
        m_lastError = "Failed to bind listening port";
        std::cerr << "[HOST] Failed to bind port " << port << ": " << m_lastError << std::endl;
        return false;
    }

    m_status = ConnectionStatus::CONNECTING;
    std::string displayIP = getLocalIP();
    std::cerr << "[HOST] Listening on port " << port << " (Local IP: " << displayIP << ")" << std::endl;
    std::cerr << "[HOST] Server started successfully. Waiting for client..." << std::endl;
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

    sf::Socket::Status status = m_listener->accept(*m_socket);
    if (status == sf::Socket::Status::Done) {
        m_status = ConnectionStatus::CONNECTED;
        m_connectionStartTime = std::chrono::steady_clock::now();
        m_lastHeartbeatReceived = m_connectionStartTime;
        std::cerr << "[HOST] Client connected from " << m_socket->getRemoteAddress().value().toString() << std::endl;
        return true;
    } else if (status == sf::Socket::Status::Error) {
        // Only log errors, not "NotReady" status (which is normal when no connection pending)
        static bool errorLogged = false;
        if (!errorLogged) {
            std::cerr << "[HOST] Error accepting connection" << std::endl;
            errorLogged = true;
        }
    }

    return false;
}

bool NetworkSession::connectToHost(const std::string& hostAddress, unsigned short port, int timeoutMs) {
    if (m_role != SessionRole::CLIENT) {
        m_lastError = "Not configured as client";
        return false;
    }

    if (timeoutMs < 0) {
        timeoutMs = NetworkConfig::getInstance().getConnectionTimeoutMs();
    }

    std::cerr << "[CLIENT] Attempting to connect to " << hostAddress << ":" << port << "..." << std::endl;
    m_socket = std::make_unique<sf::TcpSocket>();
    
    auto ipAddressOpt = sf::IpAddress::resolve(hostAddress);
    if (!ipAddressOpt.has_value()) {
        m_lastError = "Failed to resolve host address: " + hostAddress;
        m_status = ConnectionStatus::ERROR;
        std::cerr << "[CLIENT] Failed to resolve IP address: " << hostAddress << std::endl;
        return false;
    }
    sf::IpAddress ipAddress = ipAddressOpt.value();
    std::cerr << "[CLIENT] Resolved to IP: " << ipAddress.toString() << std::endl;
    
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        sf::Socket::Status status = m_socket->connect(ipAddress, port, sf::seconds(2.0f));
        
        if (status == sf::Socket::Status::Done) {
            m_socket->setBlocking(false);
            m_status = ConnectionStatus::CONNECTED;
            m_connectionStartTime = std::chrono::steady_clock::now();
            m_lastHeartbeatReceived = m_connectionStartTime;
            m_reconnectAttempts = 0;
            std::cerr << "[CLIENT] Successfully connected to host!" << std::endl;
            return true;
        }
        
        if (status == sf::Socket::Status::Error) {
            m_lastError = "Failed to connect to host: " + hostAddress + ":" + std::to_string(port) + 
                         " (Check firewall/port forwarding)";
            m_status = ConnectionStatus::ERROR;
            std::cerr << "[CLIENT] Connection error: " << m_lastError << std::endl;
            return false;
        }
        
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();
        if (elapsed > timeoutMs) {
            m_lastError = "Connection timeout after " + std::to_string(timeoutMs / 1000) + 
                         " seconds. Check:\n- IP address is correct\n- Port " + std::to_string(port) + 
                         " is forwarded on host's router\n- Firewall allows connections";
            m_status = ConnectionStatus::ERROR;
            std::cerr << "[CLIENT] " << m_lastError << std::endl;
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
        // Heartbeat packets are a single 0xFF byte; update timer and swallow them
        if (received == 1 && outData[0] == 0xFF) {
            m_lastHeartbeatReceived = std::chrono::steady_clock::now();
            return 0;
        }

        // Any other payload counts as activity for timeout purposes
        m_lastHeartbeatReceived = std::chrono::steady_clock::now();
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

// Check if connection is still alive, and try to reconnect if needed
void NetworkSession::updateConnectionTimeout() {
    // If we're connected, check if we've received a heartbeat recently
    if (m_status == ConnectionStatus::CONNECTED) {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastHeartbeat = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_lastHeartbeatReceived).count();

        // If too much time has passed without a heartbeat, connection is dead
        auto& config = NetworkConfig::getInstance();
        if (timeSinceLastHeartbeat > config.getConnectionTimeoutSeconds()) {
            disconnect();
            m_lastError = "Connection timeout - no heartbeat received";
            m_status = ConnectionStatus::ERROR;
        }
    }
    // If connection failed and we're a client, try to reconnect
    else if (m_status == ConnectionStatus::ERROR && m_role == SessionRole::CLIENT) {
        if (shouldAttemptReconnect()) {
            attemptReconnect(m_reconnectHost, m_reconnectPort);
        }
    }
}

void NetworkSession::updateHeartbeat() {
    if (m_status != ConnectionStatus::CONNECTED) {
        return;
    }

    auto& config = NetworkConfig::getInstance();
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastSent = std::chrono::duration_cast<std::chrono::seconds>(
        now - m_lastHeartbeatSent).count();

    if (timeSinceLastSent >= config.getHeartbeatIntervalSeconds()) {
        sendHeartbeat();
    }
}

void NetworkSession::disconnect() {
    if (m_socket) {
        m_socket->disconnect();
    }
    if (m_status == ConnectionStatus::CONNECTED) {
        m_status = ConnectionStatus::DISCONNECTING;
    } else {
        m_status = ConnectionStatus::DISCONNECTED;
    }
}

// Store connection info so we can try to reconnect if connection drops
void NetworkSession::setReconnectInfo(const std::string& hostAddress, unsigned short port) {
    m_reconnectHost = hostAddress;
    m_reconnectPort = port;
    m_reconnectAttempts = 0;  // Reset attempt counter
}

// Check if we should try to reconnect (haven't tried too many times, and enough time has passed)
bool NetworkSession::shouldAttemptReconnect() const {
    // Need to have a host address and port to reconnect to
    if (m_reconnectHost.empty() || m_reconnectPort == 0) {
        return false;
    }
    
    // Don't try more than the maximum number of attempts
    auto& config = NetworkConfig::getInstance();
    if (m_reconnectAttempts >= config.getMaxReconnectAttempts()) {
        return false;
    }
    
    // Wait a bit between reconnection attempts
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastAttempt = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_lastReconnectAttempt).count();
    
    // Only reconnect if enough time has passed since last attempt
    return timeSinceLastAttempt >= config.getReconnectDelayMs();
}

// Try to reconnect to the host
bool NetworkSession::attemptReconnect(const std::string& hostAddress, unsigned short port, int timeoutMs) {
    // Check if we should even try
    if (!shouldAttemptReconnect()) {
        return false;
    }
    
    // Record that we're trying to reconnect
    m_lastReconnectAttempt = std::chrono::steady_clock::now();
    m_reconnectAttempts++;
    
    // Try to connect
    m_status = ConnectionStatus::CONNECTING;
    m_lastError = "";
    
    // If connection succeeds, reset attempt counter
    if (connectToHost(hostAddress, port, timeoutMs)) {
        m_reconnectAttempts = 0;
        return true;
    }
    
    // Connection failed
    return false;
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
    // Check if we're in WSL2 - get Windows host IP from /etc/resolv.conf
    // WSL2 stores the Windows host IP as the nameserver
    FILE* resolv = fopen("/etc/resolv.conf", "r");
    std::string wslHostIP = "";
    if (resolv) {
        char line[256];
        while (fgets(line, sizeof(line), resolv)) {
            if (strncmp(line, "nameserver", 10) == 0) {
                char ip[INET_ADDRSTRLEN];
                if (sscanf(line, "nameserver %s", ip) == 1) {
                    const std::string candidate(ip);
                    const bool isLoopback = (candidate == "127.0.0.1" || candidate == "127.0.0.53");
                    const bool isPrivate = (candidate.rfind("192.168.", 0) == 0) || (candidate.rfind("10.", 0) == 0);
                    if (!isLoopback && isPrivate) {
                        wslHostIP = candidate;
                        std::cerr << "[HOST] WSL2 nameserver (Windows Wi-Fi/Ethernet) IP: " << wslHostIP << std::endl;
                        break;
                    }
                }
            }
        }
        fclose(resolv);
    }
    
    // Try using SFML's getLocalAddress() which should get the right interface
    auto localAddr = sf::IpAddress::getLocalAddress();
    if (localAddr.has_value()) {
        std::string ip = localAddr->toString();
        // Prefer nameserver-derived Windows IP only when it's a private LAN address
        if (!wslHostIP.empty()) {
            std::cerr << "[HOST] WSL2 detected - prefer Windows host IP: " << wslHostIP << " (SFML local: " << ip << ")" << std::endl;
            return wslHostIP;
        }
        // Skip loopback and WSL2 virtual network IPs (172.18-31.x.x range)
        if (ip.rfind("127.", 0) != 0 &&
            ip.find("172.18.") != 0 && ip.find("172.19.") != 0 && 
            ip.find("172.20.") != 0 && ip.find("172.21.") != 0 &&
            ip.find("172.22.") != 0 && ip.find("172.23.") != 0 &&
            ip.find("172.24.") != 0 && ip.find("172.25.") != 0 &&
            ip.find("172.26.") != 0 && ip.find("172.27.") != 0 &&
            ip.find("172.28.") != 0 && ip.find("172.29.") != 0 &&
            ip.find("172.30.") != 0 && ip.find("172.31.") != 0) {
            return ip;
        }
    }
    
    // Fallback: iterate through interfaces and find non-WSL2, non-loopback IP
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) {
        // If we have WSL2 host IP, use it
        if (!wslHostIP.empty()) {
            return wslHostIP;
        }
        return "127.0.0.1";
    }

    std::string result = "127.0.0.1";
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            std::string ifName = ifa->ifa_name;
            // Skip loopback and virtual/tunnel interfaces
            const bool isLoopback = (ifName == "lo");
            const bool isDocker = (ifName.rfind("docker", 0) == 0) || (ifName.rfind("br-", 0) == 0) || (ifName.rfind("veth", 0) == 0);
            const bool isWSL = ifName.find("wsl") != std::string::npos;
            const bool isTailscale = (ifName.rfind("tailscale", 0) == 0) || (ifName.rfind("ts", 0) == 0);
            const bool isValidLan = (ifName.rfind("eth", 0) == 0) || (ifName.rfind("en", 0) == 0) || (ifName.rfind("wl", 0) == 0) || (ifName.rfind("wlan", 0) == 0);

            if (!isLoopback && !isDocker && !isWSL && !isTailscale && isValidLan) {
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr,
                         ip, INET_ADDRSTRLEN);
                std::string ipStr = std::string(ip);
                // Prefer 192.168.x.x or 10.x.x.x (typical local network)
                // Accept 172.16-31.x.x (private), skip 127.* and 172.18-31 WSL NAT only if no better option
                const bool isPrivateA = ipStr.rfind("10.", 0) == 0;
                const bool isPrivateB = ipStr.rfind("192.168.", 0) == 0;
                const bool isPrivateC = ipStr.rfind("172.", 0) == 0 && ipStr.size() > 6 && std::stoi(ipStr.substr(4, 2)) >= 16 && std::stoi(ipStr.substr(4, 2)) <= 31;
                const bool isLoopbackIp = ipStr.rfind("127.", 0) == 0;
                if (!isLoopbackIp && (isPrivateA || isPrivateB || isPrivateC)) {
                    result = ipStr;
                    break;
                } else if (result == "127.0.0.1" && !isLoopbackIp) {
                    // Use as fallback if nothing better yet
                    result = ipStr;
                }
            }
        }
    }

    if (ifaddr) {
        freeifaddrs(ifaddr);
    }

    // If we found a private Windows host IP, prefer it over WSL NAT addresses or loopback
    if (!wslHostIP.empty()) {
        std::cerr << "[HOST] Using Windows host IP for WSL2: " << wslHostIP << std::endl;
        return wslHostIP;
    }

    // Never return loopback unless absolutely nothing else is available
    if (result.rfind("127.", 0) == 0) {
        std::cerr << "[HOST] No suitable LAN IP found; falling back to loopback " << result << std::endl;
    }

    return result;
#endif
}
