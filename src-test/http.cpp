#include "../src/HttpServer.h"
HttpServer httpServer;
int main(int argc, char* argv[])
{
	httpServer.get("/", [](Request req, Response res) {
		res.send("Hello, world!");
	});

	httpServer.listen(3000);
};