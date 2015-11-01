#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include "httpconnectionhandler.h"
#include "httprequest.h"

#include <QHostAddress>
#include <QObject>

/**
 * This class represents an HTTP Stream. In HTTP/1, this is just the whole connection, while HTTP/2 specifies streams.
 */
class HttpStream : public QObject
{
	Q_OBJECT
	
public:
	/** Creates a new (root) stream for the given protocol. */
	static HttpStream* newStream(HttpRequest::Protocol protocol, const QHostAddress &address);
	
	/** Returns the protocol of this stream. */
	HttpRequest::Protocol protocol() const { return _protocol; }
	
	/** Returns the address of the peer. */
	QHostAddress address() const { return _address; }
	
	/** Receives some data from the TCP connection. */
	virtual void recv(const QByteArray &data) = 0;
	
public slots:
	/** Can send a timeout message to the client. The connection will be closed afterwards. */
	virtual void sendTimeout() {}
	
protected:
	HttpStream(HttpRequest::Protocol protocol, const QHostAddress &address);
	
signals:
	/** Emited when the server and client agreed to change the used protocol. */
	void changeProtocol(HttpRequest::Protocol protocol);
	
private:
	/** The protocol of this stream. */
	HttpRequest::Protocol _protocol;
	
	/** The address of the peer. */
	QHostAddress _address;
	
};

// ### HTTP/1.x specific implementation #############################################################################################################

/**
 * This class is just a wrapper for a HTTP/1.x connection since HTTP/1.x doesn't support more
 * than 1 stream per connection.
 */
class Http1Stream : public HttpStream
{
	Q_OBJECT
	
public:
	/** Contains helpful stream states for parsing HTTP/1.x connections. */
	enum State
	{
		/** Initial state. */
		IDLE,
		/** Reading headers. */
		HEADERS,
		/** Reading body. */
		BODY,
		/** Responding to an request. */
		RESPONDING
	};
	
	Http1Stream(HttpRequest::Protocol protocol, const QHostAddress &address);
	
	virtual void recv(const QByteArray &data);
	
	State state() const { return _state; }
	
private:
	/** A buffer containing stuff received from the client. */
	QByteArray _buffer;
	
	/** The current request received from the client. */
	HttpRequest *_currentRequest;
	
	/** The name of the latest inserted header. */
	QByteArray _latestHeaderName;
	/** The value of the latest inserted header. */
	QByteArray _latestHeaderValue;
	
	/** The current state of this stream. */
	State _state;
	
};

// ### HTTP/2.0 specific implementation #############################################################################################################

class Http2Stream : public HttpStream
{
	Q_OBJECT
	
public:
	/** Contains the specified HTTP/2.0 stream states. */
	enum State
	{
		/** Initial state. */
		IDLE,
		/** The server put it in reserved state. */
		RESERVED_LOCAL,
		/** The client put it in reserved state. */
		RESERVED_REMOTE,
		/** Initiated with headers, data can be sent. */
		OPEN,
		/** The server sent an endstream-flag. */
		HALF_CLOSED_LOCAL,
		/** The client sent an endstream-flag. */
		HALF_CLOSED_REMOTE,
		/** Terminal state. */
		CLOSED
	};
	Q_ENUM(State)
	
	/** This class stores an HTTP/2.0 frame. */
	class Frame
	{
		
	public:
		Frame(const QByteArray &data);
		
		qint32 length() const { return _length; }
		qint8 type() const { return _type; }
		qint8 flags() const { return _flags; }
		bool reserved() const { return _reserved; }
		qint32 streamId() const { return _streamId; }
		
		QByteArray data() const { return _data; }
		void append(const QByteArray &data) { _data.append(data); }
		
	private:
		qint32 _length; // 24bit
		qint8 _type;
		qint8 _flags;
		bool _reserved;
		qint32 _streamId; // 31bit
		
		QByteArray _data;
		
	};
	
	/** Creates a new HTTP/2 Stream with the given stream id. */
	Http2Stream(HttpRequest::Protocol protocol, const QHostAddress &address, qint32 streamId = 0);
	
	virtual void recv(const QByteArray &data);
	void recvFrame(const Frame &frame);
	
	qint32 streamId() const { return _streamId; }
	State state() const { return _state; }
	
	Http2Stream *parent() { return _parent; }
	QList<Http2Stream*> children() { return _children; }
	
private:
	/** A buffer containing stuff received from the client. */
	QByteArray _buffer;
	
	/** If this is the root stream, this is used to make sure the client sent the preface. */
	bool _preface;
	
	/** The current receiving frame. */
	Frame *_currentFrame;
	
	/** The stream identification of this stream. */
	qint32 _streamId;
	/** The state of this stream. */
	State _state;
	
	Http2Stream *_parent;
	QList<Http2Stream*> _children;
	
};

#endif // HTTPSTREAM_H
