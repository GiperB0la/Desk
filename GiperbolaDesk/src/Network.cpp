#include "../include/Network.hpp"
#include "../include/ScreenViewer.hpp"


Network::Network()
    : running_(false) { }

Network::~Network()
{
    stop();
}

void Network::init(const std::string& local_ip, unsigned int local_port)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }

    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ == INVALID_SOCKET) {
        WSACleanup();
        throw std::runtime_error("Failed to create socket");
    }

    localAddr_.sin_family = AF_INET;
    localAddr_.sin_port = htons(local_port);
    if (InetPtonA(AF_INET, local_ip.c_str(), &localAddr_.sin_addr) != 1) {
        closesocket(socket_);
        WSACleanup();
        throw std::runtime_error("Invalid IP address");
    }

    if (bind(socket_, (sockaddr*)&localAddr_, sizeof(localAddr_)) == SOCKET_ERROR) {
        closesocket(socket_);
        WSACleanup();
        throw std::runtime_error("Bind failed");
    }
}

void Network::start(bool demonstration, const std::string& local_ip, unsigned int local_port,
    const std::string& ip_recipient, unsigned int port_recipient)
{
    this->local_ip = local_ip;
    this->local_port = local_port;
    this->ip_recipient = ip_recipient;
    this->port_recipient = port_recipient;

    init(local_ip, local_port);
    startReceiving();

    if (!demonstration) {
        ScreenViewer viewer_;
        while (viewer_.is_open() && running_) {
            if (!viewer_.poll_events(this)) {
                break;
            }

            if (auto frame = get_frame()) {
                viewer_.display_frame(frame);
            }
        }
    }
    else {
        while (running_) {
            sendFrame(ScreenManager::capture_screen_as_jpg());
        }
    }
}

void Network::stop()
{
    stopReceiving();
    closesocket(socket_);
    WSACleanup();
}

bool Network::sendFrame(const std::vector<uint8_t>& frame)
{
    sockaddr_in remoteAddr{};
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(port_recipient);
    if (inet_pton(AF_INET, ip_recipient.c_str(), &remoteAddr.sin_addr) <= 0) {
        return false;
    }

    static uint32_t frameId = 1;
    size_t totalChunks = (frame.size() + CHUNK_DATA_SIZE - 1) / CHUNK_DATA_SIZE;

    for (size_t i = 0; i < totalChunks; i++) {
        ChunkHeader header;
        header.frameId = htonl(frameId);
        header.chunkIndex = htons(static_cast<uint16_t>(i));
        header.totalChunks = htons(static_cast<uint16_t>(totalChunks));

        size_t offset = i * CHUNK_DATA_SIZE;
        size_t bytesLeft = frame.size() - offset;
        size_t chunkSize = std::min(bytesLeft, CHUNK_DATA_SIZE);

        std::vector<uint8_t> packet(sizeof(header) + chunkSize);
        std::memcpy(packet.data(), &header, sizeof(header));
        std::memcpy(packet.data() + sizeof(header), frame.data() + offset, chunkSize);

        int sent = sendto(socket_,
            reinterpret_cast<const char*>(packet.data()),
            static_cast<int>(packet.size()),
            0,
            reinterpret_cast<sockaddr*>(&remoteAddr),
            sizeof(remoteAddr));

        if (sent == SOCKET_ERROR) {
            return false;
        }
    }

    frameId++;
    return true;
}

bool Network::send_event(EventType event, const EventPayload& evPayload)
{
    sockaddr_in remoteAddr{};
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(port_recipient);
    if (inet_pton(AF_INET, ip_recipient.c_str(), &remoteAddr.sin_addr) <= 0) {
        return false;
    }

    std::vector<uint8_t> packet;
    packet.push_back(static_cast<uint8_t>(0xBB));
    packet.push_back(static_cast<uint8_t>(event));

    std::vector<uint8_t> payload;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, MouseMoveData> || std::is_same_v<T, MouseClickData>) {
            payload.push_back(static_cast<uint8_t>(arg.x & 0xFF));
            payload.push_back(static_cast<uint8_t>((arg.x >> 8) & 0xFF));
            payload.push_back(static_cast<uint8_t>(arg.y & 0xFF));
            payload.push_back(static_cast<uint8_t>((arg.y >> 8) & 0xFF));
        }
        else if constexpr (std::is_same_v<T, MouseWheelData>) {
            payload.push_back(static_cast<uint8_t>(arg.x & 0xFF));
            payload.push_back(static_cast<uint8_t>((arg.x >> 8) & 0xFF));
            payload.push_back(static_cast<uint8_t>(arg.y & 0xFF));
            payload.push_back(static_cast<uint8_t>((arg.y >> 8) & 0xFF));
            payload.push_back(static_cast<uint8_t>(arg.delta & 0xFF));
            payload.push_back(static_cast<uint8_t>((arg.delta >> 8) & 0xFF));
        }
        else if constexpr (std::is_same_v<T, KeyPressData>) {
            payload.push_back(static_cast<uint8_t>(arg.keycode & 0xFF));
            payload.push_back(static_cast<uint8_t>((arg.keycode >> 8) & 0xFF));
        }
        }, evPayload);

    packet.push_back(static_cast<uint8_t>(payload.size()));
    packet.insert(packet.end(), payload.begin(), payload.end());

    int sent = sendto(socket_,
        reinterpret_cast<const char*>(packet.data()),
        static_cast<int>(packet.size()),
        0,
        reinterpret_cast<sockaddr*>(&remoteAddr),
        sizeof(remoteAddr));

    return sent != SOCKET_ERROR;
}

void Network::startReceiving()
{
    if (running_) return;
    running_ = true;
    recvThread_ = std::thread(&Network::receiveLoop, this);
}

void Network::stopReceiving()
{
    if (!running_) return;
    running_ = false;

    if (socket_ != INVALID_SOCKET) {
        shutdown(socket_, SD_BOTH);
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }

    if (recvThread_.joinable())
        recvThread_.join();
}

void Network::receiveLoop()
{
    std::vector<uint8_t> buffer(4 * 1024 * 1024);
    sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);

    while (running_) {
        int received = recvfrom(socket_,
            reinterpret_cast<char*>(buffer.data()),
            static_cast<int>(buffer.size()),
            0,
            (sockaddr*)&senderAddr,
            &senderAddrSize);

        if (received > 0) {
            uint8_t firstByte = buffer[0];
            if (firstByte == 0xAA) {
                if (received < sizeof(ChunkHeader)) {
                    continue;
                }

                ChunkHeader header;
                std::memcpy(&header, buffer.data(), sizeof(header));

                header.frameId = ntohl(header.frameId);
                header.chunkIndex = ntohs(header.chunkIndex);
                header.totalChunks = ntohs(header.totalChunks);

                size_t dataSize = received - sizeof(header);
                const uint8_t* dataPtr = buffer.data() + sizeof(header);

                handleChunk(header, dataPtr, dataSize, senderAddr);
            }
            else if (firstByte == 0xBB) {
                handleEvent(buffer.data(), received, senderAddr);
            }
        }
        else if (received == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK && running_) {
                std::cerr << "recvfrom failed, error: " << err << std::endl;
            }
        }
    }
}

void Network::handleChunk(const ChunkHeader& header,
    const uint8_t* data, size_t dataSize,
    const sockaddr_in& senderAddr)
{
    auto& frame = frames_progress_[header.frameId];

    if (frame.totalChunks == 0) {
        frame.totalChunks = header.totalChunks;
        frame.chunks.resize(header.totalChunks);
    }

    if (header.chunkIndex < frame.chunks.size() &&
        frame.chunks[header.chunkIndex].empty()) {
        frame.chunks[header.chunkIndex].assign(data, data + dataSize);
        frame.receivedChunks++;
    }

    if (frame.receivedChunks == frame.totalChunks) {
        std::vector<uint8_t> fullFrame;
        for (auto& c : frame.chunks) {
            fullFrame.insert(fullFrame.end(), c.begin(), c.end());
        }

        pushFrame(std::move(fullFrame));
        frames_progress_.erase(header.frameId);
    }
}

void Network::handleEvent(const uint8_t* data, size_t size, const sockaddr_in& senderAddr)
{
    if (size < 2) return;

    EventType event = static_cast<EventType>(data[1]);
    uint8_t payloadSize = data[2];

    if (size < 2 + payloadSize) return;

    EventPayload payload;

    switch (event) {
    case EventType::MouseMove: {
        if (payloadSize != 4) break;
        int x = data[3] | (data[4] << 8);
        int y = data[5] | (data[6] << 8);
        payload = MouseMoveData{ x, y };
        commitEvent(event, payload);
        break;
    }

    case EventType::MouseLeftClick: {
        if (payloadSize != 4) break;
        int x = data[3] | (data[4] << 8);
        int y = data[5] | (data[6] << 8);
        payload = MouseClickData{ x, y };
        commitEvent(event, payload);
        break;
    }

    case EventType::MouseRightClick: {
        if (payloadSize != 4) break;
        int x = data[3] | (data[4] << 8);
        int y = data[5] | (data[6] << 8);
        payload = MouseClickData{ x, y };
        commitEvent(event, payload);
        break;
    }

    case EventType::MouseWheel: {
        if (payloadSize != 6) break;
        int x = data[3] | (data[4] << 8);
        int y = data[5] | (data[6] << 8);
        int delta = data[6] | (data[7] << 8);
        payload = MouseWheelData{ x, y, delta };
        commitEvent(event, payload);
        break;
    }

    case EventType::KeyPress: {
        if (payloadSize != 2) break;
        int keycode = data[3] | (data[4] << 8);
        payload = KeyPressData{ keycode };
        commitEvent(event, payload);
        break;
    }

    default:
        break;
    }
}

void Network::commitEvent(EventType event, const EventPayload& payload)
{
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, MouseMoveData>) {
            SetCursorPos(arg.x, arg.y);
        }
        else if constexpr (std::is_same_v<T, MouseClickData>) {
            if (event == EventType::MouseLeftClick) {
                SetCursorPos(arg.x, arg.y);
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }
            else if (event == EventType::MouseRightClick) {
                SetCursorPos(arg.x, arg.y);
                mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
            }
        }
        else if constexpr (std::is_same_v<T, MouseWheelData>) {
            SetCursorPos(arg.x, arg.y);
            mouse_event(MOUSEEVENTF_WHEEL, 0, 0, arg.delta, 0);
        }
        else if constexpr (std::is_same_v<T, KeyPressData>) {
            INPUT input[2] = {};

            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wVk = arg.keycode;

            input[1].type = INPUT_KEYBOARD;
            input[1].ki.wVk = arg.keycode;
            input[1].ki.dwFlags = KEYEVENTF_KEYUP;

            SendInput(2, input, sizeof(INPUT));
        }
        else if constexpr (std::is_same_v<T, std::monostate>) {
            return;
        }
        }, payload);
}

void Network::pushFrame(std::vector<uint8_t>&& frame)
{
    std::lock_guard<std::mutex> lock(frame_mutex_);
    frame_queue_.push(std::move(frame));
    if (frame_queue_.size() > 5) {
        frame_queue_.pop();
    }
}

std::optional<std::vector<uint8_t>> Network::get_frame()
{
    std::lock_guard<std::mutex> lock(frame_mutex_);
    if (frame_queue_.empty()) return std::nullopt;
    auto f = std::move(frame_queue_.front());
    frame_queue_.pop();
    return f;
}