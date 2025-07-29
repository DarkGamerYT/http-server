#include "WebSocket.hpp"

#include "HttpServer.hpp"

void WebSocket::send(const std::string& data) const {
    std::string frame;
    frame.push_back(static_cast<char>(0x81)); // FIN + text frame

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

    frame += data;

    HttpServer::sendToSocket(this->mClientSocket, frame);
};

void WebSocket::closeSocket() const {
#if defined(_WIN32)
    ::closesocket(this->mClientSocket);
#elif defined(__unix__) || defined(__APPLE__)
    ::close(this->mClientSocket);
#endif
};