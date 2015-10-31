#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include "httpconnectionhandler.h"

#include <QObject>

/**
 * This class represents an HTTP Stream. In HTTP/1, this is just the whole connection, while HTTP/2 specifies streams.
 */
class HttpStream : public QObject
{
	Q_OBJECT
	
public:
	/** Creates a new (root) stream for the given protocol. */
	static HttpStream* newStream(HttpConnectionHandler::Protocol protocol);
	
	/** Returns the protocol of this stream. */
	HttpConnectionHandler::Protocol protocol() const { return _protocol; }
	
	/** Receives some data from the TCP connection. */
	virtual void recv(const QByteArray &data) = 0;
	
public slots:
	/** Can send a timeout message to the client. The connection will be closed afterwards. */
	virtual void sendTimeout() {}
	
protected:
	HttpStream(HttpConnectionHandler::Protocol protocol);
	
signals:
	/** Emited when the server and client agreed to change the used protocol. */
	void changeProtocol(HttpConnectionHandler::Protocol protocol);
	
private:
	/** The protocol of this stream. */
	HttpConnectionHandler::Protocol _protocol;
	
};

class Http1Stream : public HttpStream
{
	Q_OBJECT
	
public:
	Http1Stream(HttpConnectionHandler::Protocol protocol);
	
	virtual void recv(const QByteArray &data);
	
};

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
	
	/** Creates a new HTTP/2 Stream with the given stream id. */
	Http2Stream(HttpConnectionHandler::Protocol protocol, qint32 streamId = 0);
	
	virtual void recv(const QByteArray &data);
	
	qint32 streamId() const { return _streamId; }
	State state() const { return _state; }
	
	Http2Stream *parent() { return _parent; }
	QList<Http2Stream*> children() { return _children; }
	
private:
	qint32 _streamId;
	State _state;
	
	Http2Stream *_parent;
	QList<Http2Stream*> _children;
	
};

#endif // HTTPSTREAM_H
