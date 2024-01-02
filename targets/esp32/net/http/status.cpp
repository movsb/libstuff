// Generated from /src/net/http/status.go by using ./_gen_status.bash. DO NOT EDIT.

#include "status.hpp"

namespace stuff {
namespace net {
namespace http {
namespace status {

const char* Status(Code code) {
	switch (code) {
		case Continue:                       return "100 Continue";
		case SwitchingProtocols:             return "101 Switching Protocols";
		case Processing:                     return "102 Processing";
		case EarlyHints:                     return "103 Early Hints";
		case OK:                             return "200 OK";
		case Created:                        return "201 Created";
		case Accepted:                       return "202 Accepted";
		case NonAuthoritativeInfo:           return "203 Non-Authoritative Information";
		case NoContent:                      return "204 No Content";
		case ResetContent:                   return "205 Reset Content";
		case PartialContent:                 return "206 Partial Content";
		case MultiStatus:                    return "207 Multi-Status";
		case AlreadyReported:                return "208 Already Reported";
		case IMUsed:                         return "226 IM Used";
		case MultipleChoices:                return "300 Multiple Choices";
		case MovedPermanently:               return "301 Moved Permanently";
		case Found:                          return "302 Found";
		case SeeOther:                       return "303 See Other";
		case NotModified:                    return "304 Not Modified";
		case UseProxy:                       return "305 Use Proxy";
		case TemporaryRedirect:              return "307 Temporary Redirect";
		case PermanentRedirect:              return "308 Permanent Redirect";
		case BadRequest:                     return "400 Bad Request";
		case Unauthorized:                   return "401 Unauthorized";
		case PaymentRequired:                return "402 Payment Required";
		case Forbidden:                      return "403 Forbidden";
		case NotFound:                       return "404 Not Found";
		case MethodNotAllowed:               return "405 Method Not Allowed";
		case NotAcceptable:                  return "406 Not Acceptable";
		case ProxyAuthRequired:              return "407 Proxy Authentication Required";
		case RequestTimeout:                 return "408 Request Timeout";
		case Conflict:                       return "409 Conflict";
		case Gone:                           return "410 Gone";
		case LengthRequired:                 return "411 Length Required";
		case PreconditionFailed:             return "412 Precondition Failed";
		case RequestEntityTooLarge:          return "413 Request Entity Too Large";
		case RequestURITooLong:              return "414 Request URI Too Long";
		case UnsupportedMediaType:           return "415 Unsupported Media Type";
		case RequestedRangeNotSatisfiable:   return "416 Requested Range Not Satisfiable";
		case ExpectationFailed:              return "417 Expectation Failed";
		case Teapot:                         return "418 I'm a teapot";
		case MisdirectedRequest:             return "421 Misdirected Request";
		case UnprocessableEntity:            return "422 Unprocessable Entity";
		case Locked:                         return "423 Locked";
		case FailedDependency:               return "424 Failed Dependency";
		case TooEarly:                       return "425 Too Early";
		case UpgradeRequired:                return "426 Upgrade Required";
		case PreconditionRequired:           return "428 Precondition Required";
		case TooManyRequests:                return "429 Too Many Requests";
		case RequestHeaderFieldsTooLarge:    return "431 Request Header Fields Too Large";
		case UnavailableForLegalReasons:     return "451 Unavailable For Legal Reasons";
		case InternalServerError:            return "500 Internal Server Error";
		case NotImplemented:                 return "501 Not Implemented";
		case BadGateway:                     return "502 Bad Gateway";
		case ServiceUnavailable:             return "503 Service Unavailable";
		case GatewayTimeout:                 return "504 Gateway Timeout";
		case HTTPVersionNotSupported:        return "505 HTTP Version Not Supported";
		case VariantAlsoNegotiates:          return "506 Variant Also Negotiates";
		case InsufficientStorage:            return "507 Insufficient Storage";
		case LoopDetected:                   return "508 Loop Detected";
		case NotExtended:                    return "510 Not Extended";
		case NetworkAuthenticationRequired:  return "511 Network Authentication Required";
		default:                             return "";
	}
}

const char* Text(Code code) {
	auto t = Status(code);
	return *t ? t + 4 : t;
}

} // namespace status
} // namespace http
} // namespace net
} // namespace stuff
