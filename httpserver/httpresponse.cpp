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

void HttpResponse::setHeader(QByteArray name, QByteArray value)
{
	Q_ASSERT(_sentHeaders == false);
	_headers.insert(name, value);
}

void HttpResponse::setHeader(QByteArray name, int value)
{
	Q_ASSERT(_sentHeaders == false);
	_headers.insert(name, QByteArray::number(value));
}

QMap<QByteArray, QByteArray> &HttpResponse::headers()
{
	return _headers;
}

void HttpResponse::setStatus(HttpResponseStatus status)
{
	_status = status;
}

void HttpResponse::writeHeaders()
{
	Q_ASSERT(_sentHeaders == false);
	QByteArray buffer;
	buffer.append("HTTP/1.1 ");
	buffer.append(QByteArray::number(_status.code()));
	buffer.append(' ');
	buffer.append(_status.text());
	buffer.append("\r\n");
	foreach (QByteArray name, _headers.keys())
	{
		buffer.append(name);
		buffer.append(": ");
		buffer.append(_headers.value(name));
		buffer.append("\r\n");
	}
	foreach (HttpCookie cookie, _cookies.values())
	{
		buffer.append("Set-Cookie: ");
		buffer.append(cookie.toByteArray());
		buffer.append("\r\n");
	}
	buffer.append("\r\n");
	writeToSocket(buffer);
	_sentHeaders = true;
}

bool HttpResponse::writeToSocket(const QByteArray &data)
{
	Q_UNUSED(data);
	return false;
}

void HttpResponse::write(QByteArray data, bool lastPart)
{
	Q_ASSERT(_sentLastPart == false);
	if (_sentHeaders == false)
	{
		QByteArray connectionMode = _headers.value("Connection");
		if (!_headers.contains("Content-Length") && !_headers.contains("Transfer-Encoding") && connectionMode != "close" && connectionMode != "Close")
		{
			if (!lastPart)
			{
				_stream->sendHeaders(_headers, _status);
				_headers.insert("Transfer-Encoding", "chunked");
			}
			else
			{
				_stream->sendHeaders(_headers, _status, data.size());
				_headers.insert("Content-Length", QByteArray::number(data.size()));
			}
		}
		writeHeaders();
	}
	bool chunked = _headers.value("Transfer-Encoding") == "chunked" || _headers.value("Transfer-Encoding") == "Chunked";
	if (chunked)
	{
		if (data.size() > 0)
		{
			QByteArray buffer = QByteArray::number(data.size(), 16);
			buffer.append("\r\n");
			writeToSocket(buffer);
			writeToSocket(data);
			writeToSocket("\r\n");
		}
	}
	else
		writeToSocket(data);
	if (lastPart)
	{
		if (chunked)
			writeToSocket("0\r\n\r\n");
//		else if (!_headers.contains("Content-Length"))
//			socket->disconnectFromHost();
		_sentLastPart = true;
	}
}


bool HttpResponse::hasSentLastPart() const
{
	return _sentLastPart;
}


void HttpResponse::setCookie(const HttpCookie &cookie)
{
	Q_ASSERT(_sentHeaders == false);
	if (!cookie.getName().isEmpty())
		_cookies.insert(cookie.getName(), cookie);
}

QMap<QByteArray, HttpCookie> &HttpResponse::cookies()
{
	return _cookies;
}

void HttpResponse::redirect(const QByteArray &url)
{
	setStatus(SEE_OTHER);
	setHeader("Location", url);
	write("Redirect", true);
}
