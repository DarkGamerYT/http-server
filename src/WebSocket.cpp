#include "WebSocket.hpp"

#include "HttpServer.hpp"

void WebSocket::send(const std::string& data) const {
    std::string frame;
    frame.push_back(static_cast<char>(0x81)); // FIN + text frame
    frame.push_back(static_cast<char>(data.size()));
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