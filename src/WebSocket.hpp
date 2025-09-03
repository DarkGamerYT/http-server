#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

#include <functional>
#include <string>

#include "Common.hpp"
#include "HttpRequest.hpp"

class WebSocket {
protected:
    Socket_t mClientSocket{ 0 };

public:
    explicit WebSocket(const Socket_t clientSocket)
        : mClientSocket(clientSocket) {};

    void send(const std::string& data = "") const;
    void closeSocket() const;
};

struct WebSocketHandler {
    using RequestHandlerNoNext   = std::function<void(const HttpRequest&, HttpResponse&)>;

    std::variant<Middleware, RequestHandlerNoNext> onRequest =
        [] (const HttpRequest&, HttpResponse&, const NextFn& next) {
            next();
        };
    std::function<void(WebSocket&)> onOpen = [] (WebSocket&) {};
    std::function<void(WebSocket&, const std::string&)> onMessage = [] (WebSocket&, const std::string&) {};
    std::function<void(WebSocket&)> onClose = [] (WebSocket&) {};
};

#endif //WEBSOCKET_HPP
