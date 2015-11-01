#include "httpstream.h"

#include <QBuffer>
#include <QRegularExpression>

// ### Helpful routines for binary protocols ########################################################################################################

quint8 readu8bit(const QByteArray &data)
{
	Q_ASSERT(data.length() >= 1);
	
	quint8 read = data[0];
	return read;
}

quint16 readu16bit(const QByteArray &data)
{
	Q_ASSERT(data.length() >= 2);
	
	quint16 num = 0;
	unsigned char read = data[0];
	num |= (read << 8);
	read = data[1];
	num |= read;
	return num;
}

quint32 readu24bit(const QByteArray &data)
{
	Q_ASSERT(data.length() >= 3);
	
	quint32 num = 0;
	unsigned char read = data[0];
	num |= (read << 16);
	read = data[1];
	num |= (read << 8);
	read = data[2];
	num |= read;
	return num;
}

quint32 readu31bit(const QByteArray &data)
{
	Q_ASSERT(data.length() >= 4);
	
	quint32 num = 0;
	unsigned char read = data[0];
	num |= ((read & 0b01111111) << 24);
	read = data[1];
	num |= (read << 16);
	read = data[2];
	num |= (read << 8);
	read = data[3];
	num |= read;
	return num;
}

quint32 readu32bit(const QByteArray &data)
{
	Q_ASSERT(data.length() >= 4);
	
	quint32 num = 0;
	unsigned char read = data[0];
	num |= (read << 24);
	read = data[1];
	num |= (read << 16);
	read = data[2];
	num |= (read << 8);
	read = data[3];
	num |= read;
	return num;
}

QByteArray writeu8bit(quint8 num)
{
	QByteArray data;
	data.append((char)num);
	return data;
}

QByteArray writeu16bit(quint16 num)
{
	QByteArray data;
	data.append((char)(num >> 8));
	data.append((char)num);
	return data;
}

QByteArray writeu24bit(quint32 num)
{
	QByteArray data;
	data.append((char)(num >> 16));
	data.append((char)(num >> 8));
	data.append((char)num);
	return data;
}

QByteArray writeu31bit(quint32 num)
{
	QByteArray data;
	data.append((char)(num >> 24) & 0b01111111);
	data.append((char)(num >> 16));
	data.append((char)(num >> 8));
	data.append((char)num);
	return data;
}

QByteArray writeu32bit(quint32 num)
{
	QByteArray data;
	data.append((char)(num >> 24));
	data.append((char)(num >> 16));
	data.append((char)(num >> 8));
	data.append((char)num);
	return data;
}

// ### General HttpStream implementation ############################################################################################################

HttpStream* HttpStream::newStream(QSettings *config, HttpRequest::Protocol protocol, const QHostAddress &address)
{
	switch (protocol)
	{
	case HttpRequest::HTTP_1_0:
	case HttpRequest::HTTP_1_1:
		return new Http1Stream(config, protocol, address);
	case HttpRequest::HTTP_2:
		return new Http2Stream(config, protocol, address);
	default:
		return 0;
	}
}

HttpStream::HttpStream(QSettings *settings, HttpRequest::Protocol protocol, const QHostAddress &address)
	: config(settings)
	, _protocol(protocol)
	, _address(address)
{
	qDebug() << "created new stream for protocol" << protocol;
}

// ### HTTP/1.x specific implementation #############################################################################################################

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

// ### HTTP/2.x specific implementation #############################################################################################################

Http2Stream::Frame::Frame(const QByteArray &data)
{
	Q_ASSERT(data.length() == 9);
	
	_length = readu24bit(data);
	_type = readu8bit(data.mid(3));
	_flags = readu8bit(data.mid(4));
	_streamId = readu31bit(data.mid(5));
}

Http2Stream::Frame::Frame(qint8 type, qint8 flags, qint32 streamId, const QByteArray &data)
	: _length(data.length())
	, _type(type)
	, _flags(flags)
	, _streamId(streamId)
	, _data(data)
{
}

QByteArray Http2Stream::Frame::serialize() const
{
	QByteArray b;
	b.append(writeu24bit(_data.length()));
	b.append(writeu8bit(_type));
	b.append(writeu8bit(_flags));
	b.append(writeu31bit(_streamId));
	b.append(_data);
	return b;
}

Http2Stream::PriorityFrame::PriorityFrame(bool exclusive, quint32 dependency, quint8 weight)
	: _exclusive(exclusive)
	, _dependency(dependency)
	, _weight(weight)
	, _error(NO_ERROR)
{
}

Http2Stream::PriorityFrame::PriorityFrame(const Frame &frame)
	: _error(NO_ERROR)
{
	Q_ASSERT(frame.type() == PRIORITY);
	
	// priority frames must not be on the root stream
	if (frame.streamId() == 0)
	{
		_error = PROTOCOL_ERROR;
		return;
	}
	
	// priority frames have always a length of 5 bytes
	if (frame.length() != 5)
	{
		_error = FRAME_SIZE_ERROR;
		return;
	}
	
	_exclusive = (frame.data()[0] & 0b10000000) != 0;
	_dependency = readu31bit(frame.data());
	_weight = readu8bit(frame.data().mid(4));
}

Http2Stream::Frame Http2Stream::PriorityFrame::serialize(quint32 streamId) const
{
	Frame f(PRIORITY, 0, streamId);
	QByteArray d = writeu31bit(_dependency);
	if (_exclusive)
	{
		f.append(d[0] | 0b10000000);
		f.append(d.mid(1));
	}
	else
		f.append(d);
	f.append(writeu8bit(_weight));
	return f;
}

Http2Stream::SettingsFrame::SettingsFrame()
	: _error(NO_ERROR)
{
}

Http2Stream::SettingsFrame::SettingsFrame(const Frame &frame)
	: _error(NO_ERROR)
{
	Q_ASSERT(frame.type() == SETTINGS);
	
	// settings frames must be on the root stream
	if (frame.streamId() != 0)
	{
		_error = PROTOCOL_ERROR;
		return;
	}
	
	// check whether this is an ack frame
	if ((frame.flags() & 0b00000001) != 0)
	{
		if (frame.length() != 0)
			_error = FRAME_SIZE_ERROR;
		return;
	}
	
	// check that the frame is not corrupted
	if (frame.length() % 6 != 0)
	{
		_error = FRAME_SIZE_ERROR;
		return;
	}
	
	// read each setting in the frame
	for (quint32 i = 0; i < frame.length(); i+=6)
	{
		quint16 parameter = readu16bit(frame.data().mid(i));
		quint32 value = readu32bit(frame.data().mid(i + 2));
		insert(parameter, value);
	}
}

Http2Stream::Frame Http2Stream::SettingsFrame::serialize() const
{
	Frame f(SETTINGS, 0, 0);
	for (quint16 parameter : _settings.keys())
	{
		quint32 value = _settings.value(parameter);
		f.append(writeu16bit(parameter));
		f.append(writeu32bit(value));
	}
	return f;
}

Http2Stream::Headers::Headers()
	: _end(false)
	, _complete(false)
{
}

void Http2Stream::Headers::append(const Frame &frame)
{
	// only append to uncomplete
	if (_complete)
	{
		qWarning() << "Appending a frame to completed headers";
		return;
	}
	
	// if end was not set, this could have been set now
	if (!_end)
		_end = (frame.flags() & 0b00000001) != 0;
	
	_data.append(frame.data());
	
	// check whether this was the last header/continuation frame
	if ((frame.flags() & 0b00000100) != 0)
	{
		_complete = true;
		qDebug() << "TODO: Decompress headers";
	}
}

Http2Stream::Http2Stream(QSettings *config, HttpRequest::Protocol protocol, const QHostAddress &address, quint32 streamId)
	: HttpStream(config, protocol, address)
	, _preface(streamId != 0)
	, _currentFrame(0)
	, _headers(0)
	, _streamId(streamId)
	, _weight(16)
	, _parent(0)
	, _root(0)
{
	Q_ASSERT(protocol == HttpRequest::HTTP_2);
	
	qDebug() << "Created new Http2Stream with id" << streamId;
	
	if (streamId == 0)
		_root = this;
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
		_preface = true;
		_buffer = _buffer.mid(24);
		
		// after receiving the preface, send a settings frame
		SettingsFrame settings;
		settings.insert(ENABLE_PUSH, 0); // the client isn't allowed push anything
		if (config->contains("maxConcurrentStreams"))
			settings.insert(MAX_CONCURRENT_STREAMS, config->value("maxConcurrentStreams").toUInt());
		emit send(settings.serialize().serialize());
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
	qDebug() << frame.length() << frame.type() << frame.flags() << frame.streamId() << frame.data();
	if (frame.streamId() != _streamId)
	{
		if (streamId() == 0)
		{
			Http2Stream *stream = _streams.value(frame.streamId());
			if (stream == 0)
			{
				stream = new Http2Stream(config, protocol(), address(), frame.streamId());
				stream->setParent(this);
				_streams.insert(stream->streamId(), stream);
			}
			stream->recvFrame(frame);
		}
		else
			_root->recvFrame(frame);
		return;
	}
	
	switch (frame.type())
	{
	case HEADERS: {
			if (_headers)
				break;
			QByteArray data = frame.data();
			// remove padding
			if ((frame.flags() & 0b00001000) != 0)
			{
				quint8 pad = readu8bit(data);
				data = data.mid(1);
				data = data.mid(0, data.length() - pad);
			}
			// divide data into priority and header data
			if ((_state == IDLE) || (_state == RESERVED_LOCAL) || (_state == RESERVED_REMOTE))
			{
				if ((frame.flags() & 0b00100000) != 0)
				{
					recvFrame(Frame(PRIORITY, 0, streamId(), data.mid(0, 5)));
					data = data.mid(5);
				}
				_state = OPEN;
			}
			_headers = new Headers;
			_headers->append(Frame(HEADERS, frame.flags() & 0b11010111, streamId(), data));
		}
		break;
	case PRIORITY: {
			PriorityFrame pf(frame);
			if (pf.error() != NO_ERROR)
			{
				qDebug() << "TODO: Send error" << pf.error();
				return;
			}
			if (!_root)
			{
				qCritical() << "Received priority frame without a root stream!!!";
				return;
			}
			Http2Stream *newParent = _root->_streams.value(pf.dependency());
			if (pf.exlusive())
			{
				for (Http2Stream *child : newParent->children())
					child->setParent(this);
			}
			setParent(newParent);
			_weight = pf.weight();
		}
		break;
	case SETTINGS: {
			SettingsFrame sf(frame);
			if (sf.error() != NO_ERROR)
			{
				qDebug() << "TODO: Send error" << sf.error();
				return;
			}
			qDebug() << sf.settings();
			// tell the peer that the settings have been set
			emit send(Frame(SETTINGS, 0x1, 0).serialize());
		}
		break;
	case CONTINUATION: {
			if (!_headers)
			{
				qDebug() << "TODO: Send error" << PROTOCOL_ERROR;
				return;
			}
			if (!_headers->complete())
				_headers->append(frame);
		}
		break;
	default:
		qDebug() << "Unknown frame type" << frame.type() << "received from" << address().toString();
	}
}

void Http2Stream::setParent(Http2Stream *parent)
{
	if (_parent)
		_parent->removeChild(this);
	_parent = parent;
	if (_parent)
	{
		_parent->addChild(this);
		_root = _parent->_root;
	}
}
