#include "httpconnectionhandler.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "httpstream.h"

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
	qDebug() << "created frame:"
			 << static_cast<FrameType>(type)
			 << QByteArray::number(flags, 2)
			 << streamId
			 << data;
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

void Http2Stream::Headers::append(const Frame &frame, HPACK *decode)
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
		qDebug() << "Decompressing headers";
		qDebug() << _data;
		_headers = decode->decode(_data);
		if (decode->error())
			qDebug() << "send" << COMPRESSION_ERROR;
		for (HPACKTableEntry entry : _headers)
			qDebug() << entry.name << ":" << entry.value;
	}
}

Http2Stream::Http2Stream(QSettings *config, HttpRequest::Protocol protocol, const QHostAddress &address, HttpConnectionHandler *connectionHandler, quint32 streamId)
	: HttpStream(config, protocol, address, connectionHandler)
	, _preface(streamId != 0)
	, _currentFrame(0)
	, _headers(0)
	, _streamId(streamId)
	, _state(IDLE)
	, _weight(16)
	, _parent(0)
	, _root(0)
{
	Q_ASSERT(protocol == HttpRequest::HTTP_2_0);
	
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
			connectionHandler->changeProtocol(HttpRequest::UNKNOWN);
			return;
		}
		_preface = true;
		_buffer = _buffer.mid(24);
		
		// after receiving the preface, send a settings frame
		SettingsFrame settings;
		settings.insert(ENABLE_PUSH, 0); // the client isn't allowed push anything
		if (config->contains("maxConcurrentStreams"))
			settings.insert(MAX_CONCURRENT_STREAMS, config->value("maxConcurrentStreams").toUInt());
		connectionHandler->send(settings.serialize().serialize());
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
	qDebug() << static_cast<FrameType>(frame.type())
			 << QByteArray::number(frame.flags(), 2).data()
			 << frame.streamId()
			 << frame.data();
	if (frame.streamId() != _streamId)
	{
		if (streamId() == 0)
		{
			Http2Stream *stream = _streams.value(frame.streamId());
			if (stream == 0)
			{
				stream = new Http2Stream(config, protocol(), address(), connectionHandler, frame.streamId());
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
			if ((frame.flags() & 0x8) != 0)
			{
				quint8 pad = readu8bit(data);
				data = data.mid(1);
				data = data.mid(0, data.length() - pad);
			}
			// divide data into priority and header data
			if ((_state == IDLE) || (_state == RESERVED_LOCAL) || (_state == RESERVED_REMOTE))
			{
				if ((frame.flags() & 0x20) != 0)
				{
					recvFrame(Frame(PRIORITY, 0, streamId(), data.mid(0, 5)));
					data = data.mid(5);
				}
				_state = OPEN;
			}
			else if ((frame.flags() & 0x20) != 0)
			{
				qDebug() << "send" << PROTOCOL_ERROR;
				return;
			}
			_headers = new Headers;
			_headers->append(Frame(HEADERS, frame.flags() & 0b11010111, streamId(), data), &(_root->decode));
			if (_headers->complete())
			{
				HttpRequest req(protocol(), address());
				for (HPACKTableEntry header : _headers->headers())
				{
					if (header.name == ":path")
						req.setPath(header.value);
					else if (header.name == ":method")
					{
						if (header.value == "GET")
							req.setMethod(HttpRequest::GET);
						else if (header.value == "POST")
							req.setMethod(HttpRequest::POST);
						else if (header.value == "OPTIONS")
							req.setMethod(HttpRequest::OPTIONS);
						else if (header.value == "PUT")
							req.setMethod(HttpRequest::PUT);
						else if (header.value == "DELETE")
							req.setMethod(HttpRequest::DELETE);
						else if (header.value == "HEAD")
							req.setMethod(HttpRequest::HEAD);
						else if (header.value == "TRACE")
							req.setMethod(HttpRequest::TRACE);
						else if (header.value == "CONNECT")
							req.setMethod(HttpRequest::CONNECT);
						else
						{
							connectionHandler->changeProtocol(HttpRequest::UNKNOWN);
							return;
						}
					}
					else if (header.name == ":scheme")
						qDebug() << "TODO: handle :scheme";
					else if (header.name == ":authority")
						qDebug() << "TODO: handle :authority";
					else if (header.name.startsWith(":"))
					{
						qDebug() << "send" << PROTOCOL_ERROR;
						return;
					}
					else
						req.insertHeader(header.name, header.value);
				}
				qDebug() << "processing request";
				HttpResponse response(this);
				requestHandler->service(req, response);
			}
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
			if ((frame.flags() & 0x1) == 0)
				connectionHandler->send(Frame(SETTINGS, 0x1, 0).serialize());
		}
		break;
	case CONTINUATION: {
			if (!_headers)
			{
				qDebug() << "TODO: Send error" << PROTOCOL_ERROR;
				return;
			}
			if (!_headers->complete())
				_headers->append(frame, &(_root->decode));
		}
		break;
	default:
		qDebug() << "Unknown frame type" << frame.type() << "received from" << address().toString();
	}
}

void Http2Stream::sendHeaders(const QMap<QByteArray, QByteArray> &headers, const HttpResponseStatus &status, int contentLength)
{
	QList<HPACKTableEntry> entries;
	HPACKTableEntry statusEntry{ ":status", QByteArray::number(status.code()) };
	entries << statusEntry;
	for (QByteArray key : headers.keys())
		entries << HPACKTableEntry{ key.toLower(), headers.value(key) };
	if (contentLength >= 0)
		entries << HPACKTableEntry{ "content-length", QByteArray::number(contentLength) };
	for (HPACKTableEntry entry : entries)
		qDebug() << entry.name << entry.value;
	QByteArray bytes = _root->encode.encode(entries);
	qDebug() << "encoded bytes:" << bytes;
	connectionHandler->send(Frame(HEADERS, 0x4, streamId(), bytes).serialize());
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
