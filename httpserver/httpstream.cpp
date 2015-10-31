#include "httpstream.h"

HttpStream* HttpStream::newStream(HttpConnectionHandler::Protocol protocol)
{
	switch (protocol)
	{
	case HttpConnectionHandler::HTTP_1_0:
	case HttpConnectionHandler::HTTP_1_1:
		return new Http1Stream(protocol);
	case HttpConnectionHandler::HTTP_2:
		return new Http2Stream(protocol);
	default:
		return 0;
	}
}

HttpStream::HttpStream(HttpConnectionHandler::Protocol protocol)
	: _protocol(protocol)
{
	qDebug() << "created new stream for protocol" << protocol;
}

Http1Stream::Http1Stream(HttpConnectionHandler::Protocol protocol)
	: HttpStream(protocol)
{
	Q_ASSERT((protocol == HttpConnectionHandler::HTTP_1_0) || (protocol == HttpConnectionHandler::HTTP_1_1));
}

void Http1Stream::recv(const QByteArray &data)
{
	qDebug() << data;
}

Http2Stream::Http2Stream(HttpConnectionHandler::Protocol protocol, qint32 streamId)
	: HttpStream(protocol)
	, _streamId(streamId)
{
	Q_ASSERT(protocol == HttpConnectionHandler::HTTP_2);
}

void Http2Stream::recv(const QByteArray &data)
{
	qDebug() << data;
}
