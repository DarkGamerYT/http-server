#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

#include <functional>
#include <string>

#include "HttpRequest.hpp"

class WebSocket {
protected:
    Socket_t mClientSocket;

public:
    explicit WebSocket(const Socket_t clientSocket)
        : mClientSocket(clientSocket) {};

    void send(const std::string& data = "") const;
    void closeSocket() const;
};

struct WebSocketHandler {
    std::function<void(WebSocket&)> onOpen = [] (WebSocket&) {};
    std::function<void(WebSocket&, const std::string&)> onMessage = [] (WebSocket&, const std::string&) {};
    std::function<void(WebSocket&)> onClose = [] (WebSocket&) {};
};

#endif //WEBSOCKET_HPP
