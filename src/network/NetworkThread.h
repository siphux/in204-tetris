#pragma once

#include "NetworkSession.h"
#include "Message.h"
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <memory>

// NetworkThread handles all network I/O operations in a separate thread
// This prevents network operations from blocking the main game loop
class NetworkThread {
public:
    NetworkThread();
    ~NetworkThread();

    // Start the network thread
    void start();
    
    // Stop the network thread (waits for thread to finish)
    void stop();
    
    // Check if thread is running
    bool isRunning() const { return m_running.load(); }

    // Queue a message to be sent (thread-safe)
    void queueSend(const Message& message);
    
    // Queue raw data to be sent (thread-safe)
    void queueSendRaw(const uint8_t* data, size_t size);
    
    // Check if there are received messages waiting
    bool hasReceivedMessages() const;
    
    // Get the next received message (thread-safe, returns empty if none)
    Message popReceivedMessage();
    
    // Get all received messages at once (thread-safe)
    std::vector<Message> popAllReceivedMessages();
    
    // Set the network session to use (must be called before start)
    void setNetworkSession(std::shared_ptr<NetworkSession> session);
    
    // Check connection status (thread-safe)
    bool isConnected() const;

private:
    // Main thread loop - runs in background thread
    void networkLoop();
    
    // Process all queued send operations
    void processSendQueue();
    
    // Receive messages from network
    void processReceive();
    
    // Thread object
    std::thread m_thread;
    
    // Atomic flag to control thread lifecycle
    std::atomic<bool> m_running;
    
    // Mutexes for thread-safe access to queues
    mutable std::mutex m_sendMutex;
    mutable std::mutex m_receiveMutex;
    
    // Condition variable to wake thread when there's data to send
    std::condition_variable m_sendCondition;
    
    // Queue of messages to send
    std::queue<Message> m_sendQueue;
    
    // Queue of received messages
    std::queue<Message> m_receiveQueue;
    
    // Network session (shared pointer for thread safety)
    std::shared_ptr<NetworkSession> m_session;
    
    // Mutex for session access
    mutable std::mutex m_sessionMutex;
};
