#include "WebSocket.hpp"

#include "HttpServer.hpp"

void WebSocket::sendFrame(const uint8_t opcode, std::span<const uint8_t> data) const {
    std::string frame;
    frame.push_back(static_cast<char>(0x80 | opcode));

    const size_t payloadSize = data.size();
    if (payloadSize <= 125) {
        frame.push_back(static_cast<char>(payloadSize));
    }
    else if (payloadSize <= 65535) {
        frame.push_back(126);
        frame.push_back(static_cast<char>((payloadSize >> 8) & 0xFF));
        frame.push_back(static_cast<char>(payloadSize & 0xFF));
    }
    else {
        frame.push_back(127);
        for (int i = 7; i >= 0; --i)
            frame.push_back(static_cast<char>((payloadSize >> (8 * i)) & 0xFF));
    };

    frame.append(reinterpret_cast<const char*>(data.data()), data.size());
    HttpServer::sendToSocket(this->mClientSocket, frame);
};

void WebSocket::send(const std::string& text) const {
    this->sendFrame(0x1, std::span(
        reinterpret_cast<const uint8_t*>(text.data()), text.size()
    ));
};

void WebSocket::send(const std::vector<uint8_t>& binary) const {
    this->sendFrame(0x2, binary);
};

void WebSocket::send(const std::span<const uint8_t> binary) const {
    this->sendFrame(0x2, binary);
};

void WebSocket::closeSocket() const {
#if defined(_WIN32)
    ::closesocket(this->mClientSocket);
#elif defined(__unix__) || defined(__APPLE__)
    ::close(this->mClientSocket);
#endif
};