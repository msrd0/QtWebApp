/**
  @file
  @author Stefan Frings
*/

#include "httpresponse.h"
#include "httpstream.h"

HttpResponse::HttpResponse(HttpStream *stream)
	: _stream(stream)
	, _status(OK)
	, _sentHeaders(false)
	, _sentLastPart(false)
{
	Q_ASSERT(stream);
}

HttpResponse::~HttpResponse()
{
	if (!_sentLastPart)
		write(QByteArray(), true);
}

void HttpResponse::setHeader(QByteArray name, QByteArray value)
{
	Q_ASSERT(_sentHeaders == false);
	_headers.insert(name.toLower(), value);
}

void HttpResponse::setHeader(QByteArray name, int value)
{
	Q_ASSERT(_sentHeaders == false);
	_headers.insert(name.toLower(), QByteArray::number(value));
}

QMap<QByteArray, QByteArray> &HttpResponse::headers()
{
	return _headers;
}

void HttpResponse::setStatus(HttpResponseStatus status)
{
	_status = status;
}

void HttpResponse::write(QByteArray data, bool lastPart)
{
	Q_ASSERT(!_sentLastPart);
	if (!_sentHeaders)
	{
		_stream->sendHeaders(headers(), cookies().values(), status(), lastPart?data.length():-1);
		_sentHeaders = true;
	}
	_stream->sendBody(data, lastPart);
}


bool HttpResponse::hasSentLastPart() const
{
	return _sentLastPart;
}


void HttpResponse::setCookie(const HttpCookie &cookie)
{
	Q_ASSERT(_sentHeaders == false);
	if (!cookie.name().isEmpty())
		_cookies.insert(cookie.name(), cookie);
}

QMap<QByteArray, HttpCookie> &HttpResponse::cookies()
{
	return _cookies;
}

void HttpResponse::redirect(const QByteArray &url)
{
	setStatus(SEE_OTHER);
	setHeader("Location", url);
	setHeader("Content-Type", "text/html; charset=utf-8");
	write("<html><body>Redirecting to <a href=\"" + url + "\">" + url + "</a></body></html>", true);
}
