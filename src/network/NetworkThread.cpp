#include "NetworkThread.h"
#include <chrono>
#include <iostream>

NetworkThread::NetworkThread()
    : m_running(false) {
}

NetworkThread::~NetworkThread() {
    stop();
}

void NetworkThread::start() {
    // Don't start if already running
    if (m_running.load()) {
        return;
    }
    
    m_running = true;
    
    // Create and start the background thread
    m_thread = std::thread(&NetworkThread::networkLoop, this);
}

void NetworkThread::stop() {
    // Don't stop if not running
    if (!m_running.load()) {
        return;
    }
    
    // Signal thread to stop
    m_running = false;
    
    // Wake up thread if it's waiting on condition variable
    m_sendCondition.notify_all();
    
    // Wait for thread to finish (join)
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void NetworkThread::setNetworkSession(std::shared_ptr<NetworkSession> session) {
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    m_session = session;
}

bool NetworkThread::isConnected() const {
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    if (!m_session) {
        return false;
    }
    return m_session->isConnected();
}

void NetworkThread::queueSend(const Message& message) {
    {
        // Lock mutex before modifying send queue
        std::lock_guard<std::mutex> lock(m_sendMutex);
        m_sendQueue.push(message);
    }
    
    // Notify network thread that there's data to send
    m_sendCondition.notify_one();
}

void NetworkThread::queueSendRaw(const uint8_t* data, size_t size) {
    // Create a message from raw data
    // We'll use a simple approach: create a GAME_STATE message with raw data
    Message msg(MessageType::GAME_STATE);
    msg.payload().append(data, size);
    
    queueSend(msg);
}

bool NetworkThread::hasReceivedMessages() const {
    std::lock_guard<std::mutex> lock(m_receiveMutex);
    return !m_receiveQueue.empty();
}

Message NetworkThread::popReceivedMessage() {
    std::lock_guard<std::mutex> lock(m_receiveMutex);
    
    if (m_receiveQueue.empty()) {
        // Return empty message if queue is empty
        return Message();
    }
    
    Message msg = m_receiveQueue.front();
    m_receiveQueue.pop();
    return msg;
}

std::vector<Message> NetworkThread::popAllReceivedMessages() {
    std::lock_guard<std::mutex> lock(m_receiveMutex);
    
    std::vector<Message> messages;
    
    // Move all messages from queue to vector
    while (!m_receiveQueue.empty()) {
        messages.push_back(m_receiveQueue.front());
        m_receiveQueue.pop();
    }
    
    return messages;
}

void NetworkThread::networkLoop() {
    // This function runs in the background thread
    while (m_running.load()) {
        // Process outgoing messages
        processSendQueue();
        
        // Process incoming messages
        processReceive();
        
        // Small sleep to prevent 100% CPU usage
        // This gives other threads a chance to run
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void NetworkThread::processSendQueue() {
    std::unique_lock<std::mutex> lock(m_sendMutex);
    
    // Wait for data or stop signal
    m_sendCondition.wait_for(lock, std::chrono::milliseconds(10), 
        [this] { return !m_sendQueue.empty() || !m_running.load(); });
    
    // Check if we have a session
    {
        std::lock_guard<std::mutex> sessionLock(m_sessionMutex);
        if (!m_session || !m_session->isConnected()) {
            // Clear queue if not connected
            while (!m_sendQueue.empty()) {
                m_sendQueue.pop();
            }
            return;
        }
    }
    
    // Send all queued messages
    while (!m_sendQueue.empty()) {
        Message msg = m_sendQueue.front();
        m_sendQueue.pop();
        
        // Unlock mutex while sending (network operation might take time)
        lock.unlock();
        
        // Serialize and send message
        auto serialized = msg.serialize();
        {
            std::lock_guard<std::mutex> sessionLock(m_sessionMutex);
            if (m_session && m_session->isConnected()) {
                m_session->sendPacket(serialized.data(), serialized.size());
            }
        }
        
        // Re-lock mutex for next iteration
        lock.lock();
    }
}

void NetworkThread::processReceive() {
    // Check if we have a session
    std::shared_ptr<NetworkSession> session;
    {
        std::lock_guard<std::mutex> sessionLock(m_sessionMutex);
        if (!m_session || !m_session->isConnected()) {
            return;
        }
        session = m_session;
    }
    
    // Try to receive data (non-blocking)
    uint8_t buffer[4096];
    size_t received = session->receivePacket(buffer, sizeof(buffer));
    
    if (received > 0) {
        // Deserialize message
        try {
            Message msg = Message::deserialize(buffer, received);
            
            // Add to receive queue (thread-safe)
            {
                std::lock_guard<std::mutex> lock(m_receiveMutex);
                m_receiveQueue.push(msg);
            }
        } catch (...) {
            // Ignore deserialization errors
            // In a real application, you'd want better error handling
        }
    }
}
