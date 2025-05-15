#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <string>

namespace HttpStatus
{
    enum Code
    {
        // 1XX - Informational responses
        /* This interim response indicates that the client should continue the request or ignore the response if the request is already finished. */
        Continue = 100,
        /* This code is sent in response to an `Upgrade` request header from the client and indicates the protocol the server is switching to. */
        SwitchingProtocols = 101,
        /* This code was used in WebDAV contexts to indicate that a request has been received by the server, but no status was available at the time of the response. */
        Processing = 102,
        /* This status code is primarily intended to be used with the `Link` header, letting the user agent start preloading resources while the server prepares a response or preconnect to an origin from which the page will need resources. */
        EarlyHints = 103,

        // 2XX - Successful responses
        /* The request succeeded. */
        OK = 200,
        /* The request succeeded, and a new resource was created as a result. This is typically the response sent after `POST` requests, or some `PUT` requests. */
        Created = 201,
        /* The request has been received but not yet acted upon. It is noncommittal, since there is no way in HTTP to later send an asynchronous response indicating the outcome of the request. It is intended for cases where another process or server handles the request, or for batch processing. */
        Accepted = 202,
        /* This response code means the returned metadata is not exactly the same as is available from the origin server, but is collected from a local or a third-party copy. This is mostly used for mirrors or backups of another resource. Except for that specific case, the `200 OK` response is preferred to this status. */
        NonAuthoritativeInformation = 203,
        /* There is no content to send for this request, but the headers are useful. The user agent may update its cached headers for this resource with the new ones. */
        NoContent = 204,
        /* Tells the user agent to reset the document which sent this request. */
        ResetContent = 205,
        /* This response code is used in response to a range request when the client has requested a part or parts of a resource. */
        PartialContent = 206,
        /* Conveys information about multiple resources, for situations where multiple status codes might be appropriate. */
        MultiStatus = 207,
        /* Used inside a `<dav:propstat>` response element to avoid repeatedly enumerating the internal members of multiple bindings to the same collection. */
        AlreadyReported = 208,
        /* The server has fulfilled a `GET` request for the resource, and the response is a representation of the result of one or more instance-manipulations applied to the current instance. */
        IMUsed = 226,

        // 3XX - Redirection messages
        /* In agent-driven content negotiation, the request has more than one possible response and the user agent or user should choose one of them. There is no standardized way for clients to automatically choose one of the responses, so this is rarely used. */
        MultipleChoices = 300,
        /* The URL of the requested resource has been changed permanently. The new URL is given in the response. */
        MovedPermanently = 301,
        /* This response code means that the URI of requested resource has been changed temporarily. Further changes in the URI might be made in the future, so the same URI should be used by the client in future requests. */
        Found = 302,
        /* The server sent this response to direct the client to get the requested resource at another URI with a `GET` request. */
        SeeOther = 303,
        /* This is used for caching purposes. It tells the client that the response has not been modified, so the client can continue to use the same cached version of the response. */
        NotModified = 304,
        /* Defined in a previous version of the HTTP specification to indicate that a requested response must be accessed by a proxy. It has been deprecated due to security concerns regarding in-band configuration of a proxy. */
        UseProxy = 305,
        /* The server sends this response to direct the client to get the requested resource at another URI with the same method that was used in the prior request. This has the same semantics as the `302 Found` response code, with the exception that the user agent must not change the HTTP method used: if a `POST` was used in the first request, a `POST` must be used in the redirected request. */
        TemporaryRedirect = 307,
        /* This means that the resource is now permanently located at another URI, specified by the `Location` response header. This has the same semantics as the `301 Moved Permanently` HTTP response code, with the exception that the user agent must not change the HTTP method used: if a `POST` was used in the first request, a `POST` must be used in the second request. */
        PermanentRedirect = 308,

        // 4XX - Client error responses
        /*  */
        BadRequest = 400,
        /*  */
        Unauthorized = 401,
        /*  */
        PaymentRequired = 402,
        /*  */
        Forbidden = 403,
        /*  */
        /*  */
        NotFound = 404,
        /*  */
        MethodNotAllowed = 405,
        /*  */
        NotAcceptable = 406,
        /*  */
        ProxyAuthenticationRequired = 407,
        /*  */
        RequestTimeout = 408,
        /*  */
        Conflict = 409,
        /*  */
        Gone = 410,
        /*  */
        LengthRequired = 411,
        /*  */
        PreconditionFailed = 412,
        /*  */
        ContentTooLarge = 413,
        /*  */
        PayloadTooLarge = 413,
        /*  */
        URITooLong = 414,
        /*  */
        UnsupportedMediaType = 415,
        /*  */
        RangeNotSatisfiable = 416,
        /*  */
        ExpectationFailed = 417,
        /*  */
        ImATeapot = 418,
        /*  */
        MisdirectedRequest = 421,
        /*  */
        UnprocessableContent = 422,
        /*  */
        UnprocessableEntity = 422,
        /*  */
        Locked = 423,
        /*  */
        FailedDependency = 424,
        /*  */
        TooEarly = 425,
        /*  */
        UpgradeRequired = 426,
        /*  */
        PreconditionRequired = 428,
        /*  */
        TooManyRequests = 429,
        /*  */
        RequestHeaderFieldsTooLarge = 431,
        /*  */
        UnavailableForLegalReasons = 451,

        // 5XX - Server error responses
        /* The server has encountered a situation it does not know how to handle. This error is generic, indicating that the server cannot find a more appropriate `5XX` status code to respond with. */
        InternalServerError = 500,
        /* The request method is not supported by the server and cannot be handled. The only methods that servers are required to support (and therefore that must not return this code) are `GET` and `HEAD`. */
        NotImplemented = 501,
        /* This error response means that the server, while working as a gateway to get a response needed to handle the request, got an invalid response. */
        BadGateway = 502,
        /* The server is not ready to handle the request. Common causes are a server that is down for maintenance or that is overloaded. Note that together with this response, a user-friendly page explaining the problem should be sent. This response should be used for temporary conditions and the `Retry-After` HTTP header should, if possible, contain the estimated time before the recovery of the service. The webmaster must also take care about the caching-related headers that are sent along with this response, as these temporary condition responses should usually not be cached. */
        ServiceUnavailable = 503,
        /* This error response is given when the server is acting as a gateway and cannot get a response in time. */
        GatewayTimeout = 504,
        /* The HTTP version used in the request is not supported by the server. */
        HTTPVersionNotSupported = 505,
        /* The server has an internal configuration error: during content negotiation, the chosen variant is configured to engage in content negotiation itself, which results in circular references when creating responses. */
        VariantAlsoNegotiates = 506,
        /* The method could not be performed on the resource because the server is unable to store the representation needed to successfully complete the request. */
        InsufficientStorage = 507,
        /* The server detected an infinite loop while processing the request. */
        LoopDetected = 508,
        /* The client request declares an HTTP Extension (RFC 2774) that should be used to process the request, but the extension is not supported. */
        NotExtended = 510,
        /* Indicates that the client needs to authenticate to gain network access. */
        NetworkAuthenticationRequired = 511,
    };

    inline std::string toString(Code status)
    {
        switch (status)
        {
            // 1XX - Informational responses
            case HttpStatus::Continue:           return "Continue";
            case HttpStatus::SwitchingProtocols: return "Switching Protocols";
            case HttpStatus::Processing:         return "Processing";
            case HttpStatus::EarlyHints:         return "Early Hints";

            // 2XX - Successful responses
            case HttpStatus::OK:                          return "OK";
            case HttpStatus::Created:                     return "Created";
            case HttpStatus::Accepted:                    return "Accepted";
            case HttpStatus::NonAuthoritativeInformation: return "Non-Authoritative Information";
            case HttpStatus::NoContent:                   return "No Content";
            case HttpStatus::ResetContent:                return "Reset Content";
            case HttpStatus::PartialContent:              return "Partial Content";
            case HttpStatus::MultiStatus:                 return "Multi-Status";
            case HttpStatus::AlreadyReported:             return "Already Reported";
            case HttpStatus::IMUsed:                      return "IM Used";

            // 3XX - Redirection messages
            case HttpStatus::MultipleChoices:   return "Multiple Choices";
            case HttpStatus::MovedPermanently:  return "Moved Permanently";
            case HttpStatus::Found:             return "Found";
            case HttpStatus::SeeOther:          return "See Other";
            case HttpStatus::NotModified:       return "Not Modified";
            case HttpStatus::UseProxy:          return "Use Proxy";
            case HttpStatus::TemporaryRedirect: return "Temporary Redirect";
            case HttpStatus::PermanentRedirect: return "Permanent Redirect";

            // 4XX - Client error responses
            case HttpStatus::BadRequest:                  return "Bad Request";
            case HttpStatus::Unauthorized:                return "Unauthorized";
            case HttpStatus::PaymentRequired:             return "Payment Required";
            case HttpStatus::Forbidden:                   return "Forbidden";
            case HttpStatus::NotFound:                    return "Not Found";
            case HttpStatus::MethodNotAllowed:            return "Method Not Allowed";
            case HttpStatus::NotAcceptable:               return "Not Acceptable";
            case HttpStatus::ProxyAuthenticationRequired: return "Proxy Authentication Required";
            case HttpStatus::RequestTimeout:              return "Request Timeout";
            case HttpStatus::Conflict:                    return "Conflict";
            case HttpStatus::Gone:                        return "Gone";
            case HttpStatus::LengthRequired:              return "Length Required";
            case HttpStatus::PreconditionFailed:          return "Precondition Failed";
            case HttpStatus::ContentTooLarge:             return "Content Too Large";
            case HttpStatus::URITooLong:                  return "URI Too Long";
            case HttpStatus::UnsupportedMediaType:        return "Unsupported Media Type";
            case HttpStatus::RangeNotSatisfiable:         return "Range Not Satisfiable";
            case HttpStatus::ExpectationFailed:           return "Expectation Failed";
            case HttpStatus::ImATeapot:                   return "I'm a teapot";
            case HttpStatus::MisdirectedRequest:          return "Misdirected Request";
            case HttpStatus::UnprocessableContent:        return "Unprocessable Content";
            case HttpStatus::Locked:                      return "Locked";
            case HttpStatus::FailedDependency:            return "Failed Dependency";
            case HttpStatus::TooEarly:                    return "Too Early";
            case HttpStatus::UpgradeRequired:             return "Upgrade Required";
            case HttpStatus::PreconditionRequired:        return "Precondition Required";
            case HttpStatus::TooManyRequests:             return "Too Many Requests";
            case HttpStatus::RequestHeaderFieldsTooLarge: return "Request Header Fields Too Large";
            case HttpStatus::UnavailableForLegalReasons:  return "Unavailable For Legal Reasons";

            // 5XX - Server error responses
            case HttpStatus::InternalServerError:           return "Internal Server Error";
            case HttpStatus::NotImplemented:                return "Not Implemented";
            case HttpStatus::BadGateway:                    return "Bad Gateway";
            case HttpStatus::ServiceUnavailable:            return "Service Unavailable";
            case HttpStatus::GatewayTimeout:                return "Gateway Timeout";
            case HttpStatus::HTTPVersionNotSupported:       return "HTTP Version Not Supported";
            case HttpStatus::VariantAlsoNegotiates:         return "Variant Also Negotiates";
            case HttpStatus::InsufficientStorage:           return "Insufficient Storage";
            case HttpStatus::LoopDetected:                  return "Loop Detected";
            case HttpStatus::NotExtended:                   return "Not Extended";
            case HttpStatus::NetworkAuthenticationRequired: return "Network Authentication Required";

            default: return {};
        };
    };
};

#endif // !HTTPSTATUS_HPP