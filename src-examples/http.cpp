#include "HttpServer.hpp"

HttpServer g_Server;
int main(int argc, char* argv[])
{
	g_Server.listen(8080);
};