#include "HttpServer.hpp"

HttpServer g_Server;
int main(int argc, char* argv[])
{
    g_Server.use("/", HttpMethod::GET,
        [](const HttpRequest& request, HttpResponse response) {
            response.setHeader("Content-Type", "text/plain");
            response.send("Hello, world!");
        }
    );

    g_Server.use(R"(/test/.*)", HttpMethod::GET,
        HttpServer::useStatic("test"));

    g_Server.use(R"(/.*)", HttpMethod::GET,
        [](const HttpRequest& request, HttpResponse response) {
            response.setHeader("Content-Type", "text/plain");
        
            std::stringstream stream;
            stream
                << "Route: " << request.getPath() << ", "
                << "IP: " << request.getRemoteAddr();
            response.send(stream.str());
        }
    );

    g_Server.listen(8000);
};
