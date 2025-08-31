#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <map>
#include <queue>
#include <mutex>
#include <optional>
#include "Protocol.hpp"

#pragma comment(lib, "ws2_32.lib")

#undef min
#undef max

#include "ScreenManager.hpp"
#include <SFML/Graphics.hpp>

struct ChunkHeader 
{
    const uint8_t magic = 0xAA;
    uint32_t frameId;
    uint16_t chunkIndex;
    uint16_t totalChunks;
};

struct FrameAssembly
{
    std::vector<std::vector<uint8_t>> chunks;
    size_t receivedChunks = 0;
    size_t totalChunks = 0;
};

constexpr size_t CHUNK_DATA_SIZE = 1400;

class Network
{
public:
    Network(unsigned short localPort);
    ~Network();

public:
    void start(const std::string& arg, const std::string& ip_recipient, int port_recipient);
    bool send_event(EventType event, const EventPayload& payload);

private:
    bool sendFrame(const std::vector<uint8_t>& frame);
    void startReceiving();
    void stopReceiving();
    void receiveLoop();
    void handleChunk(const ChunkHeader& header, const uint8_t* data, size_t dataSize, const sockaddr_in& senderAddr);
    void handleEvent(const uint8_t* data, size_t size, const sockaddr_in& senderAddr);
    void pushFrame(std::vector<uint8_t>&& frame);
    void commitEvent(EventType event, const EventPayload& payload);
    std::optional<std::vector<uint8_t>> get_frame();

private:
    SOCKET socket_;
    sockaddr_in localAddr_;
    std::thread recvThread_;
    std::atomic<bool> running_;
    std::map<uint32_t, FrameAssembly> frames_progress_;
    std::mutex frame_mutex_;
    std::queue<std::vector<uint8_t>> frame_queue_;

    std::string ip_recipient;
    int port_recipient;
};