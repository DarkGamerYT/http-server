#include "WebSocket.hpp"

#include "HttpServer.hpp"

void WebSocket::send(const std::string& data) const {
    std::string frame;
    frame.push_back(static_cast<char>(0x81)); // FIN + text frame
    frame += static_cast<char>(data.size());
    frame += data;

    HttpServer::sendToSocket(this->m_ClientSocket, frame);
};

void WebSocket::closeSocket() const {
#if defined(_WIN32)
    ::closesocket(this->m_ClientSocket);
#elif defined(__unix__) || defined(__APPLE__)
    ::close(this->m_ClientSocket);
#endif
};