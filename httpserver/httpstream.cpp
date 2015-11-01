#include "httpstream.h"

#include <QBuffer>
#include <QRegularExpression>

HttpStream* HttpStream::newStream(HttpRequest::Protocol protocol, const QHostAddress &address)
{
	switch (protocol)
	{
	case HttpRequest::HTTP_1_0:
	case HttpRequest::HTTP_1_1:
		return new Http1Stream(protocol, address);
	case HttpRequest::HTTP_2:
		return new Http2Stream(protocol, address);
	default:
		return 0;
	}
}

HttpStream::HttpStream(HttpRequest::Protocol protocol, const QHostAddress &address)
	: _protocol(protocol)
{
	qDebug() << "created new stream for protocol" << protocol;
}

// ### HTTP/1.x specific implementation #############################################################################################################

Http1Stream::Http1Stream(HttpRequest::Protocol protocol, const QHostAddress &address)
	: HttpStream(protocol, address)
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

// ### HTTP/2.x specific implementation #############################################################################################################

Http2Stream::Frame::Frame(const QByteArray &data)
{
	Q_ASSERT(data.length() == 9);
	
	_length = 0;
	_length |= (data[0] << 16);
	_length |= (data[1] << 8);
	_length |= data[2];
	
	_type = data[3];
	_flags = data[4];
	
	_reserved = (data[5] & 0b10000000) != 0;
	_streamId = 0;
	_streamId |= ((data[5] & 0b01111111) << 24);
	_streamId |= (data[6] << 16);
	_streamId |= (data[7] << 8);
	_streamId |= data[8];
}

Http2Stream::Http2Stream(HttpRequest::Protocol protocol, const QHostAddress &address, qint32 streamId)
	: HttpStream(protocol, address)
	, _preface(streamId != 0)
	, _currentFrame(0)
	, _streamId(streamId)
{
	Q_ASSERT(protocol == HttpRequest::HTTP_2);
}

void Http2Stream::recv(const QByteArray &data)
{
	_buffer.append(data);
	if (!_preface && (_buffer.size() < 24))
		return;
	if (!_preface)
	{
		if (!_buffer.startsWith("PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"))
		{
			emit changeProtocol(HttpRequest::UNKNOWN);
			return;
		}
		_buffer = _buffer.mid(24);
	}
	
	while (!_buffer.isEmpty())
	{
		if (_currentFrame)
		{
			int remaining = _currentFrame->length() - _currentFrame->data().length();
			if (remaining > 0)
			{
				QByteArray read = _buffer.mid(0, qMin(remaining, _buffer.length()));
				_currentFrame->append(read);
				_buffer = _buffer.mid(read.length());
			}
			else
			{
				recvFrame(*_currentFrame);
				delete _currentFrame;
				_currentFrame = 0;
			}
		}
		
		if (!_currentFrame)
		{
			if (_buffer.size() >= 9)
			{
				_currentFrame = new Frame(_buffer.mid(0, 9));
				_buffer = _buffer.mid(9);
			}
			else
				break;
		}
	}
}

void Http2Stream::recvFrame(const Frame &frame)
{
	qDebug() << frame.length() << frame.type() << frame.flags() << frame.reserved() << frame.streamId() << frame.data();
}
