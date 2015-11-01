#ifndef HTTPSTREAM_H
#define HTTPSTREAM_H

#include "httpconnectionhandler.h"
#include "httprequest.h"

#include <QHostAddress>
#include <QObject>

/** Reads an unsigned 8bit integer from data. */
extern quint8 readu8bit(const QByteArray &data);
/** Reads an unsigned 16bit integer from data. */
extern quint16 readu16bit(const QByteArray &data);
/** Reads an unsigned 24bit integer from data. */
extern quint32 readu24bit(const QByteArray &data);
/** Reads an unsigned 31bit integer from data. */
extern quint32 readu31bit(const QByteArray &data);
/** Reads an unsigned 32bit integer from data. */
extern quint32 readu32bit(const QByteArray &data);

/** Writes an unsigned 8bit integer. */
extern QByteArray writeu8bit(quint8 num);
/** Writes an unsigned 16bit integer. */
extern QByteArray writeu16bit(quint16 num);
/** Writes an unsigned 24bit integer. */
extern QByteArray writeu24bit(quint32 num);
/** Writes an unsigned 31bit integer. */
extern QByteArray writeu31bit(quint32 num);
/** Writes an unsigned 32bit integer. */
extern QByteArray writeu32bit(quint32 num);

/**
 * This class represents an HTTP Stream. In HTTP/1, this is just the whole connection, while HTTP/2 specifies streams.
 */
class HttpStream : public QObject
{
	Q_OBJECT
	
public:
	/** Creates a new (root) stream for the given protocol. */
	static HttpStream* newStream(QSettings *config, HttpRequest::Protocol protocol, const QHostAddress &address);
	
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
	HttpStream(QSettings *settings, HttpRequest::Protocol protocol, const QHostAddress &address);
	
	/** Configuration settings. */
	QSettings *config;
	
signals:
	/** Emited when the server should sent data to the client. */
	void send(const QByteArray &data);
	
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
	
	Http1Stream(QSettings *config, HttpRequest::Protocol protocol, const QHostAddress &address);
	
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
	
	/** All specified HTTP/2.0 error codes. */
	enum ErrorCode
	{
		NO_ERROR = 0x0,
		PROTOCOL_ERROR = 0x1,
		INTERNAL_ERROR = 0x2,
		FLOW_CONTROL_ERROR = 0x3,
		SETTINGS_TIMEOUT = 0x4,
		STREAM_CLOSED = 0x5,
		FRAME_SIZE_ERROR = 0x6,
		REFUSED_STREAM = 0x7,
		CANCEL = 0x8,
		COMPRESSION_ERROR = 0x9,
		CONNECT_ERROR = 0xa,
		ENHANCE_YOUR_CALM = 0xb,
		INADEQUATE_SECURITY = 0xc,
		HTTP_1_1_REQUIRED = 0xd
	};
	Q_ENUM(ErrorCode)
	
	/** All specified HTTP/2.0 frame types. */
	enum FrameType
	{
		DATA = 0x0,
		HEADERS = 0x1,
		PRIORITY = 0x2,
		RST_STREAM = 0x3,
		SETTINGS = 0x4,
		PUSH_PROMISE = 0x5,
		PING = 0x6,
		GOAWAY = 0x7,
		WINDOW_UPDATE = 0x8,
		CONTINUATION = 0x9
	};
	Q_ENUM(FrameType)
	
	/** All specified HTTP/2.0 SETTINGS Parameters. */
	enum SettingsParameter
	{
		/** Allows the sender to inform the remote endpoint of the maximum size of the header
		 * compression table used to decode header blocks, in octets. */
		HEADER_TABLE_SIZE = 0x1,
		/** This setting can be used to disable server push. */
		ENABLE_PUSH = 0x2,
		/** Indicates the maximum number of concurrent streams that the sender will allow. This
		 * limit is directional: it applies to the number of streams that the sender permits
		 * the receiver to create. */
		MAX_CONCURRENT_STREAMS = 0x3,
		/** Indicates the sender's initial window size (in octets) for stream-level flow control. */
		INITIAL_WINDOW_SIZE = 0x4,
		/** Indicates the size of the largest frame payload that the sender is willing to receive,
		 * in octets. */
		MAX_FRAME_SIZE = 0x5,
		/** This advisory setting informs a peer of the maximum size of header list that the sender
		 * is prepared to accept, in octets. The value is based on the uncompressed size of header
		 * fields, including the length of the name and value in octets plus an overhead of 32 octets
		 * for each header field. */
		MAX_HEADER_LIST_SIZE = 0x6
	};
	Q_ENUM(SettingsParameter)
	
	/** This class stores an HTTP/2.0 frame. */
	class Frame
	{
		
	public:
		/** Reads the Frame metadata from the 9 bytes. */
		Frame(const QByteArray &data);
		/** Creates a new Frame with the given data. */
		Frame(qint8 type, qint8 flags, qint32 streamId, const QByteArray &data = QByteArray());
		
		/** Returns the length of this frame. This does not work with frames not read from raw data! */
		quint32 length() const { return _length; }
		/** Returns the type of this frame. */
		quint8 type() const { return _type; }
		/** Returns the flags of this frame. */
		quint8 flags() const { return _flags; }
		/** Returns the stream id of this frame. */
		quint32 streamId() const { return _streamId; }
		
		/** Returns the data of this frame. */
		QByteArray data() const { return _data; }
		/** Appends the bytes to the data of this frame. */
		void append(const QByteArray &data) { _data.append(data); _length = _data.length(); }
		/** Appends the bytes to the data of this frame. */
		void append(char data) { _data.append(data); _length = _data.length(); }
		
		/** Serializes this frame to raw data. */
		QByteArray serialize() const;
		
	private:
		quint32 _length; // 24bit
		quint8 _type;
		quint8 _flags;
		quint32 _streamId; // 31bit
		
		QByteArray _data;
		
	};
	
	/** This class is used to read and serialize priority frames. */
	class PriorityFrame
	{
		
	public:
		PriorityFrame(bool exclusive, quint32 dependency, quint8 weight);
		PriorityFrame(const Frame &frame);
		
		/** Returns true if the stream should be the only child of its parent. */
		bool exlusive() const { return _exclusive; }
		/** Returns the dependency parent stream identifier. */
		quint32 dependency() const { return _dependency; }
		/** Returns the weight of this stream. */
		quint8 weight() const { return _weight; }
		
		/** Returns the error processed while parsing the data. */
		ErrorCode error() const { return _error; }
		
		/** Serializes this priority frame into an HTTP/2.0 frame. */
		Frame serialize(quint32 streamId) const;
		
	private:
		bool _exclusive;
		quint32 _dependency;
		quint8 _weight;
		ErrorCode _error;
		
	};
	
	/** This class is used to read and serialize settings frames. */
	class SettingsFrame
	{
		
	public:
		SettingsFrame();
		SettingsFrame(const Frame &frame);
		
		/** Return all settings set in this frame. */
		QMap<quint16, quint32> settings() const { return _settings; }
		/** Insert or update a setting into this frame. */
		void insert(quint16 parameter, quint32 value) { _settings.insert(parameter, value); }
		
		/** Returns the error processed while parsing the data. */
		ErrorCode error() const { return _error; }
		
		/** Serializes this settings frame into an HTTP/2.0 frame. */
		Frame serialize() const;
		
	private:
		QMap<quint16, quint32> _settings;
		ErrorCode _error;
		
	};
	
	/** This class is used to read header and continuation frames. */
	class Headers
	{
		
	public:
		Headers();
		
		void append(const Frame &frame);
		
		/** Returns the data of the appended Header/Continuation frames. The output is only usefull if the completed flag is set. */
		QByteArray data() const { return _data; }
		/** Returns if the END_STREAM flag was set on any appended frame. */
		bool end() const { return _end; }
		/** Returns whether all required frames have been appended. If true, please don't append frames anymore. */
		bool complete() const { return _complete; }
		
	private:
		QByteArray _data;
		bool _end;
		bool _complete;
		
	};
	
	/** Creates a new HTTP/2 Stream with the given stream id. */
	Http2Stream(QSettings *config, HttpRequest::Protocol protocol, const QHostAddress &address, quint32 streamId = 0);
	
	virtual void recv(const QByteArray &data);
	void recvFrame(const Frame &frame);
	
	/** Returns the stream identifier of this stream. */
	quint32 streamId() const { return _streamId; }
	/** Returns the state of this stream. Note that the root stream is always idle. */
	State state() const { return _state; }
	
	/** Returns the dependency parent stream or 0 if there is no parent stream. */
	Http2Stream *parent() { return _parent; }
	/** Returns a list of all dependency children of this stream. */
	QSet<Http2Stream*> children() { return _children; }
	
	/** Sets the dependency parent of this stream. */
	void setParent(Http2Stream *parent);
	/** Adds the dependency child to this stream. */
	void addChild(Http2Stream *child) { _children.insert(child); }
	/** Remove the dependency child from this stream. */
	bool removeChild(Http2Stream *child) { return _children.remove(child); }
	
private:
	/** A buffer containing stuff received from the client. */
	QByteArray _buffer;
	
	/** If this is the root stream, this is used to make sure the client sent the preface. */
	bool _preface;
	
	/** The current receiving frame. */
	Frame *_currentFrame;
	
	/** The header data received from the client. */
	Headers *_headers;
	
	/** The stream identification of this stream. */
	quint32 _streamId;
	/** The state of this stream. */
	State _state;
	/** The weight of this stream. */
	quint8 _weight;
	
	Http2Stream *_parent;
	Http2Stream *_root;
	QSet<Http2Stream*> _children;
	QMap<quint32, Http2Stream*> _streams; // only on root stream
	
};

#endif // HTTPSTREAM_H
