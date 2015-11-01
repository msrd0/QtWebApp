#include "httpstream.h"

#include <QBuffer>
#include <QRegularExpression>

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
