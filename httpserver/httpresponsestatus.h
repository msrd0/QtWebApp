/*
 * 
 * 
 * Copyright (C) 2015 Dominic S. Meiser <meiserdo@web.de>
 * 
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or any later
 * version.
 * 
 * This work is distributed in the hope that it will be useful, but without
 * any warranty; without even the implied warranty of merchantability or
 * fitness for a particular purpose. See version 2 and version 3 of the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HTTPSTATUSCODES_H
#define HTTPSTATUSCODES_H

#include "httpglobal.h"
#include "httprequest.h"

#include <QSharedPointer>

class DECLSPEC HttpResponseStatus : public QObject
{
	Q_OBJECT
	
public:
	HttpResponseStatus(int code, const QByteArray &text, HttpRequest::Protocol since);
	HttpResponseStatus(int code, const QByteArray &text, HttpRequest::Protocol since, const HttpResponseStatus &fallback);
	HttpResponseStatus(const HttpResponseStatus &other);
	void operator= (const HttpResponseStatus &other);
	
	int code() const { return _code; }
	QByteArray text() const { return _text; }
	HttpRequest::Protocol since() const { return _since; }
	
	HttpResponseStatus status (HttpRequest::Protocol protocol) const;
	
private:
	int _code;
	QByteArray _text;
	HttpRequest::Protocol _since;
	QSharedPointer<HttpResponseStatus> _fallback;
};

#define STATUS(code, text, protocol) HttpResponseStatus((code), (text), HttpRequest::protocol)
#define STATUS_FALLBACK(code, text, protocol, fallback) HttpResponseStatus((code), (text), HttpRequest::protocol, (fallback))

//// HTTP/1.0 100 Continue
//#define CONTINUE STATUS(100, "Continue", HTTP_1_0)
//// HTTP/1.0 101 Switching Protocols
//#define SWITCHING_PROTOCOLS STATUS(101, "Switching Protocols", HTTP_1_0)
//// HTTP/1.0 200 OK
//#define OK STATUS(200, "OK", HTTP_1_0)
//// HTTP/1.0 201 Created
//#define CREATED STATUS(201, "Created", HTTP_1_0)
//// HTTP/1.0 202 Accepted
//#define ACCEPTED STATUS(202, "Accepted", HTTP_1_0)
//// HTTP/1.1 203 Non Authoritative Information
//#define NON_AUTHORITATIVE_INFORMATION STATUS(203, "Non Authoritative Information", HTTP_1_1)
//// HTTP/1.0 204 No Content
//#define NO_CONTENT STATUS(204, "No Content", HTTP_1_0)
//// HTTP/1.0 205 Reset Content
//#define RESET_CONTENT STATUS(205, "Reset Content", HTTP_1_0)
//// HTTP/1.0 206 Partial Content
//#define PARTIAL_CONTENT STATUS(206, "Partial Content", HTTP_1_0)
//// HTTP/1.0 300 Multiple Choices
//#define MULTIPLE_CHOICES STATUS(300, "Multiple Choices", HTTP_1_0)
//// HTTP/1.0 301 Moved Permanently
//#define MOVED_PERMANENTLY STATUS(301, "Moved Permanently", HTTP_1_0)
//// HTTP/1.0 302 Found
//#define FOUND STATUS(302, "Found", HTTP_1_0)
//// HTTP/1.1 303 See Other
//#define SEE_OTHER STATUS(303, "See Other", HTTP_1_1)
//// HTTP/1.0 304 Not Modified
//#define NOT_MODIFIED STATUS(304, "Not Modified", HTTP_1_0)
//// HTTP/1.1 305 Use Proxy
//#define USE_PROXY STATUS(305, "Use Proxy", HTTP_1_1)
//// HTTP/1.0 306 Switch Proxy
//#define SWITCH_PROXY STATUS(306, "Switch Proxy", HTTP_1_0)
//// HTTP/1.1 307 Temporary Redirect
//#define TEMPORARY_REDIRECT STATUS(307, "Temporary Redirect", HTTP_1_1)
//// HTTP/1.1 308 Permanent Redirect
//#define PERMANENT_REDIRECT STATUS(308, "Permanent Redirect", HTTP_1_1)
//// HTTP/1.0 400 Bad Request
//#define BAD_REQUEST STATUS(400, "Bad Request", HTTP_1_0)
//// HTTP/1.0 401 Unauthorized
//#define UNAUTHORIZED STATUS(401, "Unauthorized", HTTP_1_0)
//// HTTP/1.0 402 Payment Required
//#define PAYMENT_REQUIRED STATUS(402, "Payment Required", HTTP_1_0)
//// HTTP/1.0 403 Forbidden
//#define FORBIDDEN STATUS(403, "Forbidden", HTTP_1_0)
//// HTTP/1.0 404 Not Found
//#define NOT_FOUND STATUS(404, "Not Found", HTTP_1_0)
//// HTTP/1.0 405 Method Not Allowed
//#define METHOD_NOT_ALLOWED STATUS(405, "Method Not Allowed", HTTP_1_0)
//// HTTP/1.0 406 Not Acceptable
//#define NOT_ACCEPTABLE STATUS(406, "Not Acceptable", HTTP_1_0)
//// HTTP/1.0 407 Proxy Authentication Required
//#define PROXY_AUTHENTICATION_REQUIRED STATUS(407, "Proxy Authentication Required", HTTP_1_0)
//// HTTP/1.0 408 Request Timeout
//#define REQUEST_TIMEOUT STATUS(408, "Request Timeout", HTTP_1_0)
//// HTTP/1.0 409 Conflict
//#define CONFLICT STATUS(409, "Conflict", HTTP_1_0)
//// HTTP/1.0 410 Gone
//#define GONE STATUS(410, "Gone", HTTP_1_0)
//// HTTP/1.0 411 Length Required
//#define LENGTH_REQUIRED STATUS(411, "Length Required", HTTP_1_0)
//// HTTP/1.0 412 Precondition Failed
//#define PRECONDITION_FAILED STATUS(412, "Precondition Failed", HTTP_1_0)
//// HTTP/1.0 413 Request Entity Too Large
//#define REQUEST_ENTITY_TOO_LARGE STATUS(413, "Request Entity Too Large", HTTP_1_0)
//// HTTP/1.0 414 Request URI Too Long
//#define REQUEST_URI_TOO_LONG STATUS(414, "Request URI Too Long", HTTP_1_0)
//// HTTP/1.0 415 Unsupperted Media Type
//#define UNSUPPERTED_MEDIA_TYPE STATUS(415, "Unsupperted Media Type", HTTP_1_0)
//// HTTP/1.0 416 Request Range Not Satisfiable
//#define REQUEST_RANGE_NOT_SATISFIABLE STATUS(416, "Request Range Not Satisfiable", HTTP_1_0)
//// HTTP/1.0 417 Expectation Failed
//#define EXPECTATION_FAILED STATUS(417, "Expectation Failed", HTTP_1_0)
//// HTTP/2.0 421 Misdirected Request
//#define MISDIRECTED_REQUEST STATUS(421, "Misdirected Request", HTTP_2_0)
//// HTTP/1.0 426 Upgrade Required
//#define UPGRADE_REQUIRED STATUS(426, "Upgrade Required", HTTP_1_0)
//// HTTP/1.0 428 Precondition Required
//#define PRECONDITION_REQUIRED STATUS(428, "Precondition Required", HTTP_1_0)
//// HTTP/1.0 429 Too Many Request
//#define TOO_MANY_REQUEST STATUS(429, "Too Many Request", HTTP_1_0)
//// HTTP/1.0 413 Request Header Fields Too Large
//#define REQUEST_HEADER_FIELDS_TOO_LARGE STATUS(413, "Request Header Fields Too Large", HTTP_1_0)
//// HTTP/1.0 500 Internal Server Error
//#define INTERNAL_SERVER_ERROR STATUS(500, "Internal Server Error", HTTP_1_0)
//// HTTP/1.0 501 Not Implemented
//#define NOT_IMPLEMENTED STATUS(501, "Not Implemented", HTTP_1_0)
//// HTTP/1.0 502 Bad Gateway
//#define BAD_GATEWAY STATUS(502, "Bad Gateway", HTTP_1_0)
//// HTTP/1.0 503 Service Unavailable
//#define SERVICE_UNAVAILABLE STATUS(503, "Service Unavailable", HTTP_1_0)
//// HTTP/1.0 504 Gateway Timeout
//#define GATEWAY_TIMEOUT STATUS(504, "Gateway Timeout", HTTP_1_0)
//// HTTP/1.0 505 HTTP Version Not Supported
//#define HTTP_VERSION_NOT_SUPPORTED STATUS(505, "HTTP Version Not Supported", HTTP_1_0)

// HTTP/1.0 200 OK
#define OK STATUS(200, "OK", HTTP_1_0)
// HTTP/1.0 201 Created
#define CREATED STATUS(201, "Created", HTTP_1_0)
// HTTP/1.0 202 Accepted
#define ACCEPTED STATUS(202, "Accepted", HTTP_1_0)
// HTTP/1.1 203 Non Authoritative Information
#define NON_AUTHORITATIVE_INFORMATION STATUS_FALLBACK(203, "Non Authoritative Information", HTTP_1_1, STATUS(200, "OK", HTTP_1_0))
// HTTP/1.0 204 No Content
#define NO_CONTENT STATUS(204, "No Content", HTTP_1_0)
// HTTP/1.0 205 Reset Content
#define RESET_CONTENT STATUS(205, "Reset Content", HTTP_1_0)
// HTTP/1.0 206 Partial Content
#define PARTIAL_CONTENT STATUS(206, "Partial Content", HTTP_1_0)
// HTTP/1.0 400 Bad Request
#define BAD_REQUEST STATUS(400, "Bad Request", HTTP_1_0)
// HTTP/1.0 401 Unauthorized
#define UNAUTHORIZED STATUS(401, "Unauthorized", HTTP_1_0)
// HTTP/1.0 402 Payment Required
#define PAYMENT_REQUIRED STATUS(402, "Payment Required", HTTP_1_0)
// HTTP/1.0 403 Forbidden
#define FORBIDDEN STATUS(403, "Forbidden", HTTP_1_0)
// HTTP/1.0 404 Not Found
#define NOT_FOUND STATUS(404, "Not Found", HTTP_1_0)
// HTTP/1.0 405 Method Not Allowed
#define METHOD_NOT_ALLOWED STATUS(405, "Method Not Allowed", HTTP_1_0)
// HTTP/1.0 406 Not Acceptable
#define NOT_ACCEPTABLE STATUS(406, "Not Acceptable", HTTP_1_0)
// HTTP/1.0 407 Proxy Authentication Required
#define PROXY_AUTHENTICATION_REQUIRED STATUS(407, "Proxy Authentication Required", HTTP_1_0)
// HTTP/1.0 408 Request Timeout
#define REQUEST_TIMEOUT STATUS(408, "Request Timeout", HTTP_1_0)
// HTTP/1.0 409 Conflict
#define CONFLICT STATUS(409, "Conflict", HTTP_1_0)
// HTTP/1.0 410 Gone
#define GONE STATUS(410, "Gone", HTTP_1_0)
// HTTP/1.0 411 Length Required
#define LENGTH_REQUIRED STATUS(411, "Length Required", HTTP_1_0)
// HTTP/1.0 412 Precondition Failed
#define PRECONDITION_FAILED STATUS(412, "Precondition Failed", HTTP_1_0)
// HTTP/1.0 413 Request Header Fields Too Large
#define REQUEST_HEADER_FIELDS_TOO_LARGE STATUS(413, "Request Header Fields Too Large", HTTP_1_0)
// HTTP/1.0 414 Request URI Too Long
#define REQUEST_URI_TOO_LONG STATUS(414, "Request URI Too Long", HTTP_1_0)
// HTTP/1.0 415 Unsupperted Media Type
#define UNSUPPERTED_MEDIA_TYPE STATUS(415, "Unsupperted Media Type", HTTP_1_0)
// HTTP/1.0 416 Request Range Not Satisfiable
#define REQUEST_RANGE_NOT_SATISFIABLE STATUS(416, "Request Range Not Satisfiable", HTTP_1_0)
// HTTP/1.0 417 Expectation Failed
#define EXPECTATION_FAILED STATUS(417, "Expectation Failed", HTTP_1_0)
// HTTP/1.0 100 Continue
#define CONTINUE STATUS(100, "Continue", HTTP_1_0)
// HTTP/1.0 101 Switching Protocols
#define SWITCHING_PROTOCOLS STATUS(101, "Switching Protocols", HTTP_1_0)
// HTTP/2.0 421 Misdirected Request
#define MISDIRECTED_REQUEST STATUS_FALLBACK(421, "Misdirected Request", HTTP_2_0, STATUS(400, "Bad Request", HTTP_1_0))
// HTTP/1.0 426 Upgrade Required
#define UPGRADE_REQUIRED STATUS(426, "Upgrade Required", HTTP_1_0)
// HTTP/1.0 300 Multiple Choices
#define MULTIPLE_CHOICES STATUS(300, "Multiple Choices", HTTP_1_0)
// HTTP/1.0 428 Precondition Required
#define PRECONDITION_REQUIRED STATUS(428, "Precondition Required", HTTP_1_0)
// HTTP/1.0 301 Moved Permanently
#define MOVED_PERMANENTLY STATUS(301, "Moved Permanently", HTTP_1_0)
// HTTP/1.0 429 Too Many Request
#define TOO_MANY_REQUEST STATUS(429, "Too Many Request", HTTP_1_0)
// HTTP/1.0 302 Found
#define FOUND STATUS(302, "Found", HTTP_1_0)
// HTTP/1.1 303 See Other
#define SEE_OTHER STATUS_FALLBACK(303, "See Other", HTTP_1_1, STATUS(302, "Found", HTTP_1_0))
// HTTP/1.0 304 Not Modified
#define NOT_MODIFIED STATUS(304, "Not Modified", HTTP_1_0)
// HTTP/1.1 305 Use Proxy
#define USE_PROXY STATUS(305, "Use Proxy", HTTP_1_1)
// HTTP/1.0 306 Switch Proxy
#define SWITCH_PROXY STATUS(306, "Switch Proxy", HTTP_1_0)
// HTTP/1.1 307 Temporary Redirect
#define TEMPORARY_REDIRECT STATUS_FALLBACK(307, "Temporary Redirect", HTTP_1_1, STATUS(302, "Found", HTTP_1_0))
// HTTP/1.1 308 Permanent Redirect
#define PERMANENT_REDIRECT STATUS_FALLBACK(308, "Permanent Redirect", HTTP_1_1, STATUS(301, "Moved Permanently", HTTP_1_0))
// HTTP/1.0 500 Internal Server Error
#define INTERNAL_SERVER_ERROR STATUS(500, "Internal Server Error", HTTP_1_0)
// HTTP/1.0 501 Not Implemented
#define NOT_IMPLEMENTED STATUS(501, "Not Implemented", HTTP_1_0)
// HTTP/1.0 502 Bad Gateway
#define BAD_GATEWAY STATUS(502, "Bad Gateway", HTTP_1_0)
// HTTP/1.0 503 Service Unavailable
#define SERVICE_UNAVAILABLE STATUS(503, "Service Unavailable", HTTP_1_0)
// HTTP/1.0 504 Gateway Timeout
#define GATEWAY_TIMEOUT STATUS(504, "Gateway Timeout", HTTP_1_0)
// HTTP/1.0 505 HTTP Version Not Supported
#define HTTP_VERSION_NOT_SUPPORTED STATUS(505, "HTTP Version Not Supported", HTTP_1_0)

#endif // HTTPSTATUSCODES_H

