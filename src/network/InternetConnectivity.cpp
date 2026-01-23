#include "InternetConnectivity.h"
#include <SFML/Network.hpp>
#include <sstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>

std::string InternetConnectivity::s_publicIP = "";
std::future<std::string> InternetConnectivity::s_fetchFuture;
bool InternetConnectivity::s_isFetching = false;

std::string InternetConnectivity::fetchPublicIPFromService() {
    struct Service {
        const char* host;
        const char* path;
    };
    
    Service services[] = {
        {"api.ipify.org", "/"},
        {"icanhazip.com", "/"},
        {"ifconfig.me", "/ip"}
    };
    
    for (const Service& service : services) {
        try {
            sf::Http http(service.host);
            sf::Http::Request request(service.path, sf::Http::Request::Method::Get);
            
            sf::Http::Response response = http.sendRequest(request, sf::seconds(3));
            
            if (response.getStatus() == sf::Http::Response::Status::Ok) {
                std::string ip = response.getBody();
                ip.erase(std::remove_if(ip.begin(), ip.end(), 
                    [](char c) { return std::isspace(c); }), ip.end());
                
                if (!ip.empty() && ip.find('.') != std::string::npos) {
                    return ip;
                }
            }
        } catch (...) {
            continue;
        }
    }
    
    return "";
}

std::string InternetConnectivity::getPublicIP() {
    if (!s_publicIP.empty()) {
        return s_publicIP;
    }
    
    if (!s_isFetching) {
        startFetchingPublicIP();
    }
    
    if (s_fetchFuture.valid()) {
        auto status = s_fetchFuture.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready) {
            s_publicIP = s_fetchFuture.get();
            s_isFetching = false;
            return s_publicIP;
        }
    }
    
    return "";
}

bool InternetConnectivity::isPublicIPReady() {
    if (!s_publicIP.empty()) {
        return true;
    }
    
    if (s_fetchFuture.valid()) {
        auto status = s_fetchFuture.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready) {
            s_publicIP = s_fetchFuture.get();
            s_isFetching = false;
            return true;
        }
    }
    
    return false;
}

void InternetConnectivity::startFetchingPublicIP() {
    if (s_isFetching) {
        return;
    }
    
    s_isFetching = true;
    s_fetchFuture = std::async(std::launch::async, fetchPublicIPFromService);
}

std::string InternetConnectivity::getConnectionInfo(const std::string& localIP, unsigned short port) {
    std::string publicIP = getPublicIP();
    
    std::ostringstream oss;
    if (!publicIP.empty()) {
        oss << publicIP << ":" << port;
    } else {
        oss << localIP << ":" << port;
        oss << " (or use your public IP if port forwarded)";
    }
    
    return oss.str();
}

std::string InternetConnectivity::getPortForwardingInstructions(unsigned short port) {
    std::ostringstream oss;
    oss << "To play over the internet:\n";
    oss << "1. Forward port " << port << " on your router\n";
    oss << "2. Share your public IP with your friend\n";
    oss << "3. Common router addresses: 192.168.1.1 or 192.168.0.1\n";
    oss << "4. Look for 'Port Forwarding' or 'Virtual Server' settings\n";
    oss << "5. Forward TCP port " << port << " to this computer's local IP";
    return oss.str();
}
