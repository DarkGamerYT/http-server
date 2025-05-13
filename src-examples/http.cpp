#include "HttpServer.hpp"

HttpServer g_Server;
int main(int argc, char* argv[])
{
	g_Server.use("/", HttpMethod::GET, [](HttpRequest request, HttpResponse response) {
		response.setHeader("Content-Type", "text/plain");
		response.send("Hello, world!");
	});

	g_Server.listen(8080);
};