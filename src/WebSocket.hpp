#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

#include <functional>
#include <string>

#include "HttpRequest.hpp"

class WebSocket {
protected:
    Socket_t m_ClientSocket;

public:
    explicit WebSocket(Socket_t clientSocket) : m_ClientSocket(clientSocket) {};

    void send(const std::string& data = "") const;
    void closeSocket() const;
};

struct WebSocketHandler {
    std::function<void(WebSocket&)> onOpen;
    std::function<void(WebSocket&, const std::string&)> onMessage;
    std::function<void(WebSocket&)> onClose;
};

#endif //WEBSOCKET_HPP
