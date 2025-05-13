#ifndef HTTPMETHOD_HPP
#define HTTPMETHOD_HPP

#include <string>
#include <algorithm>

#undef DELETE
namespace HttpMethod
{
    // HTTP methods:
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods
    enum Method {
        /* Requests a representation of the specified resource. Requests using GET should only retrieve data and should not contain a request content. */
        GET = 0,
        /* Asks for a response identical to a GET request, but without a response body. */
        HEAD,
        /* Submits an entity to the specified resource, often causing a change in state or side effects on the server. */
        POST,
        /* Replaces all current representations of the target resource with the request content. */
        PUT,
        /* Deletes the specified resource. */
        DELETE,
        /* Establishes a tunnel to the server identified by the target resource. */
        CONNECT,
        /* Describes the communication options for the target resource. */
        OPTIONS,
        /* Performs a message loop-back test along the path to the target resource. */
        TRACE,
        /* Applies partial modifications to a resource. */
        PATCH
    };

    inline std::string toString(Method method)
    {
        switch (method)
        {
            case HttpMethod::GET:     return "GET";
            case HttpMethod::HEAD:    return "HEAD";
            case HttpMethod::POST:    return "POST";
            case HttpMethod::PUT:     return "PUT";
            case HttpMethod::DELETE:  return "DELETE";
            case HttpMethod::CONNECT: return "CONNECT";
            case HttpMethod::OPTIONS: return "OPTIONS";
            case HttpMethod::TRACE:   return "TRACE";
            case HttpMethod::PATCH:   return "PATCH";

            default: return std::string();
        };
    };

    inline HttpMethod::Method fromString(const std::string& method)
    {
        std::string methodString;
        std::transform(method.begin(), method.end(), std::back_inserter(methodString),
            [](char c) { return toupper(c); });

        if (methodString == "GET") {
            return HttpMethod::GET;
        }
        else if (methodString == "HEAD") {
            return HttpMethod::HEAD;
        }
        else if (methodString == "POST") {
            return HttpMethod::POST;
        }
        else if (methodString == "PUT") {
            return HttpMethod::PUT;
        }
        else if (methodString == "DELETE") {
            return HttpMethod::DELETE;
        }
        else if (methodString == "CONNECT") {
            return HttpMethod::CONNECT;
        }
        else if (methodString == "OPTIONS") {
            return HttpMethod::OPTIONS;
        }
        else if (methodString == "TRACE") {
            return HttpMethod::TRACE;
        }
        else if (methodString == "PATCH") {
            return HttpMethod::PATCH;
        }
        else {
            throw std::invalid_argument("Unexpected HTTP method");
        };
    };
};

#endif // !HTTPMETHOD_HPP
