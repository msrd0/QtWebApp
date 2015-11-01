#include "httpstream.h"

#include <QBuffer>
#include <QRegularExpression>

Http1Stream::Http1Stream(QSettings *config, HttpRequest::Protocol protocol, const QHostAddress &address)
	: HttpStream(config, protocol, address)
	, _currentRequest(0)
	, _state(IDLE)
{
	Q_ASSERT((protocol == HttpRequest::HTTP_1_0) || (protocol == HttpRequest::HTTP_1_1));
}

void Http1Stream::recv(const QByteArray &data)
{
	_buffer.append(data);
	QBuffer buf(&_buffer);
	buf.open(QIODevice::ReadOnly);
	QByteArray line;
	while (!(line = buf.readLine()).isEmpty())
	{
		switch (_state)
		{
		case IDLE:
			{
				Q_ASSERT(!_currentRequest);
				_currentRequest = new HttpRequest(protocol(), address());
				QRegularExpression regex("(?P<method>GET|POST|OPTIONS|PUT|DELETE|HEAD|TRACE|CONNECT)\\s+(?<path>\\S+)\\s+HTTP/1.([01])\r\n");
				QRegularExpressionMatch match = regex.match(line);
				if (!match.hasMatch())
				{
					emit changeProtocol(HttpRequest::UNKNOWN);
					return;
				}
				QString method = match.captured("method");
				if (method == "GET")
					_currentRequest->setMethod(HttpRequest::GET);
				else if (method == "POST")
					_currentRequest->setMethod(HttpRequest::POST);
				else if (method == "OPTIONS")
					_currentRequest->setMethod(HttpRequest::OPTIONS);
				else if (method == "PUT")
					_currentRequest->setMethod(HttpRequest::PUT);
				else if (method == "DELETE")
					_currentRequest->setMethod(HttpRequest::DELETE);
				else if (method == "HEAD")
					_currentRequest->setMethod(HttpRequest::HEAD);
				else if (method == "TRACE")
					_currentRequest->setMethod(HttpRequest::TRACE);
				else if (method == "CONNECT")
					_currentRequest->setMethod(HttpRequest::CONNECT);
				else
				{
					emit changeProtocol(HttpRequest::UNKNOWN);
					return;
				}
				_currentRequest->setPath(match.captured("path"));
			}
			_state = HEADERS;
			break;
			
		case HEADERS:
			{
				int colon = line.indexOf(':');
				if (colon > 0)
				{
					if (!_latestHeaderName.isEmpty())
						_currentRequest->insertHeader(_latestHeaderName, _latestHeaderValue);
					_latestHeaderName = line.mid(0, colon);
					_latestHeaderValue = line.mid(colon + 1).trimmed();
				}
				else if (line.trimmed().isEmpty())
				{
					if (!_latestHeaderName.isEmpty())
						_currentRequest->insertHeader(_latestHeaderName, _latestHeaderValue);
					_state = BODY;
				}
				else
				{
					_latestHeaderValue += line.trimmed();
				}
			}
			break;
		}
	}
}
