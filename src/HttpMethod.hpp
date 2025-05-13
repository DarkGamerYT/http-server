#ifndef HTTPMETHOD_HPP
#define HTTPMETHOD_HPP

#include <string>

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
};

#endif // !HTTPMETHOD_HPP
