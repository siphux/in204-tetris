#pragma once
#include <string>
#include <future>

class InternetConnectivity {
public:
    static std::string getPublicIP();
    static bool isPublicIPReady();
    static std::string getConnectionInfo(const std::string& localIP, unsigned short port);
    static void startFetchingPublicIP();
    static std::string getPortForwardingInstructions(unsigned short port);

private:
    static std::string s_publicIP;
    static std::future<std::string> s_fetchFuture;
    static bool s_isFetching;
    
    static std::string fetchPublicIPFromService();
};
