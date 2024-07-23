#include "Request.h"
inline std::string getMethod(std::string data);
inline std::string getRoute(std::string data);
inline std::string getBody(std::string data);
inline std::map<std::string, std::string> getHeaders(std::string data);

Request::Request(std::string data)
{
	this->mRoute = ::getRoute(data);
	this->mMethod = ::getMethod(data);
	this->mBody = ::getBody(data);
	this->mHeaders = ::getHeaders(data);
};

inline std::string getMethod(std::string data)
{
	std::string method;
	std::string::size_type pos = data.find(" ");
	method = data.substr(0, pos);

	return method;
};

inline std::string getRoute(std::string data)
{
	std::string method = getMethod(data);
	std::string path = data.substr(method.length() + 1);
	std::string::size_type pos = path.find("HTTP/");
	if (pos != std::string::npos)
		path = path.substr(0, pos - 1);
	else path = "/";

	return path;
};

inline bool isNumber(const std::string& s) {
	return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
};

inline std::string getBody(std::string data)
{
	std::string body;
	std::string contentLength = getHeaders(data)["Content-Length"];
	bool isNumber = ::isNumber(contentLength);
	if (true == isNumber)
	{
		int length = std::stoi(contentLength);
		std::string::size_type pos = data.find("\r\n\r\n");
		if (pos != std::string::npos)
			body = data.substr(pos + 4, length);
	};

	return body;
};

inline std::map<std::string, std::string> getHeaders(std::string data)
{
	std::map<std::string, std::string> m;

	std::istringstream resp(data);
	std::string header;
	std::string::size_type index;
	while (std::getline(resp, header) && header != "\r")
	{
		index = header.find(":", 0);
		if (index != std::string::npos)
		{
			std::string headerKey = header.substr(0, index);
			std::string headerValue = header.substr(index + 1);

			String::trim(headerKey);
			String::trim(headerValue);
			m.insert(std::make_pair(
				headerKey,
				headerValue
			));
		};
	};

	return m;
};