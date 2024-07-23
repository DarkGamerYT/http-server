#include "Response.h"
std::ostringstream Response::_write()
{
	std::string statusMessage = HttpStatus::reasonPhrase(this->statusCode);

	std::ostringstream stream;
	stream
		<< "HTTP/1.1 " << std::to_string(this->statusCode)
		<< " " << statusMessage << "\r\n";

	for (const auto& [key, value] : this->headers)
		stream << key << ": " << value << "\r\n";
	return stream;
};

bool Response::redirect(std::string location, int status)
{
	this->statusCode = status;
	return this->redirect(location);
};

bool Response::redirect(std::string location)
{
	if (!HttpStatus::isRedirection(this->statusCode))
		this->statusCode = 302;
	std::string statusMessage = HttpStatus::reasonPhrase(this->statusCode);

	std::ostringstream stream;
	stream
		<< "HTTP/1.1 " << std::to_string(this->statusCode)
		<< " " << statusMessage << "\r\n";
	stream << "Location: " << location;

	std::string str = stream.str();
	return this->_send(str);
};

bool Response::send(std::string data)
{
	if (this->headers.find("Content-Type") == this->headers.end())
		this->setHeader("Content-Type", "text/html");

	size_t length = data.length();
	this->setHeader("Content-Length", std::to_string(length));
	
	if (HttpStatus::isInformational(this->statusCode) ||
		204 == this->statusCode ||
		304 == this->statusCode)
	{
		this->removeHeader("Content-Type");
		this->removeHeader("Content-Length");
		this->removeHeader("Transfer-Encoding");
		data = "";
	};

	if (205 == this->statusCode) {
		this->removeHeader("Transfer-Encoding");
		data = "";
	};

	std::ostringstream stream = this->_write();
	std::string requestMethod = this->mRequest.method();
	if (requestMethod != "HEAD" &&
		(requestMethod != "CONNECT" && HttpStatus::isSuccessful(this->statusCode)))
		stream << "\r\n" << data;

	return this->_send(stream.str());
};

bool Response::_send(std::string data)
{
	if (true == this->mHeadersSent)
		return false;

#if defined(_WIN32)
	int bytesSent;
	long totalBytesSent = 0;
	while (totalBytesSent < data.size())
	{
		bytesSent = ::send(this->mClientSocket, data.c_str(), data.size(), 0);
		if (bytesSent < 0)
			break;

		totalBytesSent += bytesSent;
	};
#elif defined(__linux__)
	long bytesSent = ::write(this->mClientSocket, data.c_str(), data.size());
#endif

	this->mHeadersSent = true;
	this->_closeSocket();
	return true;
};

void Response::_closeSocket()
{
#if defined(_WIN32)
	closesocket(this->mClientSocket);
#elif defined(__linux__)
	close(this->mClientSocket);
#endif
};

bool Response::sendFile(std::string path)
{
	std::ifstream stream(path);
	if (!std::filesystem::exists(path))
	{
		this->headers.clear();

		this->setHeader("Content-Type", "text/plain");
		this->send("404 - File not found.");
		return false;
	};
	
	std::stringstream buffer;
	buffer << stream.rdbuf();
	stream.close();

	std::string str = buffer.str();
	return this->send(str);
};