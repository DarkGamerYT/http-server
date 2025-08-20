#include <print>

#include "HttpServer.hpp"
HttpServer g_Server{ true, HttpVersion::HTTP_1_1 };

int main(int argc, char* argv[])
{
    g_Server.use("/", HttpMethod::GET,
        [](const HttpRequest& request, HttpResponse& response, const NextFn& next) {
            response.setHeader("Content-Type", "text/plain");
            response.send("Hello, world!");
        }
    );

    g_Server.websocket("/ws", {
        .onOpen = [](const WebSocket& ws) {
            std::println("New WebSocket connection");
            ws.send("Hello, world!");
        },
        .onMessage = [](const WebSocket& ws, const std::string& msg) {
            std::println("Received message: '{}'", msg);
        },
        .onClose = [](const WebSocket& ws) {
            std::println("WebSocket closed");
        },
    });

    g_Server.listen(8000);
};
