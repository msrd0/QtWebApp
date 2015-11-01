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


QByteArray HttpRequest::getHeader(const QByteArray &name) const
{
	return _headers.value(name);
}

QList<QByteArray> HttpRequest::getHeaders(const QByteArray &name) const
{
	return _headers.values(name);
}

QMultiMap<QByteArray, QByteArray> HttpRequest::getHeaderMap() const
{
	return _headers;
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

HttpRequest::~HttpRequest()
{
	foreach (QByteArray key, _uploadedFiles.keys())
	{
		QTemporaryFile *file = _uploadedFiles.value(key);
		file->close();
		delete file;
	}
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

