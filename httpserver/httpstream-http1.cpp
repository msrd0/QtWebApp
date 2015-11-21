#include "httpconnectionhandler.h"
#include "httprequesthandler.h"
#include "httpstream.h"

#include <QBuffer>
#include <QRegularExpression>

Http1Stream::Http1Stream(QSettings *config, HttpRequest::Protocol protocol, const QHostAddress &address, HttpConnectionHandler *connectionHandler)
	: HttpStream(config, protocol, address, connectionHandler)
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
		qDebug() << "Http1Stream::recv: state:" << state() << "; line:" << line;
		switch (_state)
		{
		case IDLE:
			{
				if (_currentRequest)
					delete _currentRequest;
				
				_currentRequest = new HttpRequest(protocol(), address());
				QRegularExpression regex("(?P<method>GET|POST|OPTIONS|PUT|DELETE|HEAD|TRACE|CONNECT)\\s+(?<path>\\S+)\\s+HTTP/1.([01])\r\n");
				QRegularExpressionMatch match = regex.match(line);
				if (!match.hasMatch())
				{
					connectionHandler->changeProtocol(HttpRequest::UNKNOWN);
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
					connectionHandler->changeProtocol(HttpRequest::UNKNOWN);
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
					switch(_currentRequest->method())
					{
					case HttpRequest::POST:
					case HttpRequest::OPTIONS:
					case HttpRequest::PUT:
						if (_currentRequest->getHeaderMap().keys().contains("Content-Length"))
						{
							_state = BODY;
							break;
						}
					default:
						_state = RESPONDING;
					}
				}
				else
				{
					_latestHeaderValue += line.trimmed();
				}
			}
			break;
		case BODY:
			qDebug() << "TODO: Accept http 1 body";
			break;
		default:
			qWarning() << "received data in a state where no data was expected";
			return;
		}
	}
	_buffer = _buffer.mid(buf.pos());
	
	if (_state == RESPONDING)
	{
		HttpResponse response(this);
		requestHandler->service(*_currentRequest, response);
	}
}

void Http1Stream::sendHeaders(const QMap<QByteArray, QByteArray> &headers, const HttpResponseStatus &status, int contentLength)
{
	QByteArray b;
	HttpResponseStatus s = status.status(protocol());
	b.append(QByteArray("HTTP/1.") + (protocol()==HttpRequest::HTTP_1_0 ? "0" : "1") + " " + QByteArray::number(s.code()) + " " + s.text() + "\r\n");
	QMap<QByteArray, QByteArray> h(headers);
	if (contentLength > 0)
		h.insert("content-length", QByteArray::number(contentLength));
	else if (!h.contains("content-length"))
		h.insert("transfer-encoding", "chunked");
	_chunked = h["transfer-encoding"].contains("chunked");
	for (QByteArray key : h.keys())
		b.append(key + ": " + h[key] + "\r\n");
	b.append("\r\n");
	connectionHandler->send(b);
}

void Http1Stream::sendBody(const QByteArray &data, bool lastPart)
{
	QByteArray b = data;
	if (_chunked)
		b = QByteArray::number(data.length()) + "\r\n" + b;
	if (lastPart && _chunked)
		b += "0\r\n";
	connectionHandler->send(b);
	
	if (lastPart)
	{
		qDebug() << "todo: close conn if requested";
		_state = IDLE;
	}
}
