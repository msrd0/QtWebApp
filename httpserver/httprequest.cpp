/**
  @file
  @author Stefan Frings
*/

#include "httpcookie.h"
#include "httprequest.h"
#include <QDir>
#include <QList>
#include <QHostAddress>

HttpRequest::HttpRequest(Protocol protocol, const QHostAddress &address)
	: _protocol(protocol)
	, _address(address)
{
	Q_ASSERT(protocol != UNKNOWN);
}

HttpRequest::~HttpRequest()
{
	foreach (QByteArray key, _uploadedFiles.keys())
	{
		QTemporaryFile *file = _uploadedFiles.value(key);
		file->close();
		delete file;
	}
}

QString HttpRequest::methodStr() const
{
	switch (method())
	{
	case GET:
		return "GET";
	case POST:
		return "POST";
	case HEAD:
		return "HEAD";
	case OPTIONS:
		return "OPTIONS";
	case PUT:
		return "PUT";
	case DELETE:
		return "DELETE";
	case TRACE:
		return "TRACE";
	case CONNECT:
		return "CONNECT";
	default:
		return QString();
	}
}

QString HttpRequest::protocolStr() const
{
	switch (protocol())
	{
	case HTTP_1_0:
		return "HTTP/1.0";
	case HTTP_1_1:
		return "HTTP/1.1";
	case HTTP_2_0:
		return "HTTP/2.0";
	default:
		return QString();
	}
}

QByteArray HttpRequest::getHeader(const QByteArray &name) const
{
	return _headers.value(name.toLower());
}

QList<QByteArray> HttpRequest::getHeaders(const QByteArray &name) const
{
	return _headers.values(name.toLower());
}

QMultiMap<QByteArray, QByteArray> HttpRequest::getHeaderMap() const
{
	return _headers;
}

void HttpRequest::insertHeader(const QByteArray &name, const QByteArray &value)
 {
//	qDebug() << name << value;
	_headers.insert(name.toLower(), value);
}

QByteArray HttpRequest::getParameter(const QByteArray &name) const
{
	return _parameters.value(name);
}

QList<QByteArray> HttpRequest::getParameters(const QByteArray &name) const
{
	return _parameters.values(name);
}

QMultiMap<QByteArray, QByteArray> HttpRequest::getParameterMap() const
{
	return _parameters;
}

QByteArray HttpRequest::getBody() const
{
	return _bodyData;
}

QTemporaryFile *HttpRequest::getUploadedFile(const QByteArray fieldName)
{
	return _uploadedFiles.value(fieldName);
}

QByteArray HttpRequest::getCookie(const QByteArray &name) const
{
	return _cookies.value(name);
}

/** Get the map of cookies */
QMap<QByteArray, QByteArray> &HttpRequest::getCookieMap()
{
	return _cookies;
}

