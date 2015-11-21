/**
  @file
  @author Stefan Frings
*/

#include "httpconnectionhandler.h"
#include "httpresponse.h"
#include "httpstream.h"
#include <QHostAddress>
#include <QRegularExpression>

HttpConnectionHandler::HttpConnectionHandler(QSettings *settings, HttpRequestHandler *requestHandler, QSslConfiguration *sslConfiguration)
	: QThread()
	, settings(settings)
	, protocol(HttpRequest::UNKNOWN)
	, rootStream(0)
	, _requestHandler(requestHandler)
	, busy(false)
	, sslConfiguration(sslConfiguration)
{
	Q_ASSERT(settings != 0);
	Q_ASSERT(requestHandler != 0);
	
	// Create TCP or SSL socket
	createSocket();
	
	// execute signals in my own thread
	moveToThread(this);
	socket->moveToThread(this);
	readTimer.moveToThread(this);
	
	// Connect signals
	connect(socket, SIGNAL(readyRead()), SLOT(read()));
	connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
	connect(&readTimer, SIGNAL(timeout()), SLOT(readTimeout()));
	readTimer.setSingleShot(true);
	
	this->start();
}


HttpConnectionHandler::~HttpConnectionHandler()
{
	quit();
	wait();
	delete socket;
	if (rootStream)
		delete rootStream;
}


void HttpConnectionHandler::createSocket()
{
	// If SSL is supported and configured, then create an instance of QSslSocket
#ifndef QT_NO_OPENSSL
	if (sslConfiguration)
	{
		QSslSocket *sslSocket = new QSslSocket();
		sslSocket->setSslConfiguration(*sslConfiguration);
		socket = sslSocket;
		qDebug("HttpConnectionHandler (%p): SSL is enabled", this);
		return;
	}
#endif
	// else create an instance of QTcpSocket
	socket = new QTcpSocket();
}


void HttpConnectionHandler::run()
{
	try
	{
		exec();
	}
	catch (...)
	{
		qCritical("HttpConnectionHandler (%p): an uncaught exception occured in the thread", this);
	}
	socket->close();
}


void HttpConnectionHandler::handleConnection(tSocketDescriptor socketDescriptor)
{
	busy = true;
	Q_ASSERT(socket->isOpen() == false); // if not, then the handler is already busy
	
	//UGLY workaround - we need to clear writebuffer before reusing this socket
	//https://bugreports.qt-project.org/browse/QTBUG-28914
	socket->connectToHost("", 0);
	socket->abort();
	
	if (!socket->setSocketDescriptor(socketDescriptor))
	{
		qCritical("HttpConnectionHandler (%p): cannot initialize socket: %s", this, qPrintable(socket->errorString()));
		return;
	}
	
#ifndef QT_NO_OPENSSL
	// Switch on encryption, if SSL is configured
	if (sslConfiguration)
	{
		qDebug("HttpConnectionHandler (%p): Starting encryption", this);
		((QSslSocket *)socket)->startServerEncryption();
	}
#endif
	
	// Start timer for read timeout
	int readTimeout = settings->value("readTimeout", 10000).toInt();
	readTimer.start(readTimeout);
}


bool HttpConnectionHandler::isBusy()
{
	return busy;
}

void HttpConnectionHandler::setBusy()
{
	this->busy = true;
}

void HttpConnectionHandler::readTimeout()
{
	if (rootStream)
		rootStream->sendTimeout();
	
	socket->flush();
	socket->disconnectFromHost();
	delete rootStream;
	rootStream = 0;
}

void HttpConnectionHandler::disconnected()
{
	qDebug() << "disconnected";
	socket->close();
	readTimer.stop();
	busy = false;
}

void HttpConnectionHandler::changeProtocol(HttpRequest::Protocol protocol)
{
	qDebug() << "TODO: Change Protocol to" << protocol;
}

bool HttpConnectionHandler::send(const QByteArray &data)
{
//	qDebug() << "sending" << data;
	int remaining = data.size();
	const char *ptr = data.data();
	while (socket->isOpen() && remaining > 0)
	{
		// Wait until the previous buffer content is written out, otherwise it could become very large
		socket->waitForBytesWritten(-1);
		int written = socket->write(ptr, remaining);
		if (written == -1)
			return false;
		ptr += written;
		remaining -= written;
	}
	return true;
}

void HttpConnectionHandler::read()
{
	// The loop adds support for HTTP pipelinig
	while (socket->bytesAvailable())
	{
		// if no stream has been created, do so
		if (!rootStream)
		{
			QByteArray line = socket->readLine();
			QRegularExpression http1("(GET|POST|OPTIONS|PUT|DELETE|HEAD|TRACE|CONNECT)\\s+\\S+\\s+HTTP/1.(?P<version>[01])\r\n");
			QRegularExpressionMatch match = http1.match(line);
			if (match.hasMatch())
			{
				if (match.captured("version") == "0")
					protocol = HttpRequest::HTTP_1_0;
				else
					protocol = HttpRequest::HTTP_1_1;
			}
			else if (line == "PRI * HTTP/2.0\r\n")
				protocol = HttpRequest::HTTP_2_0;
			rootStream = HttpStream::newStream(settings, protocol, socket->peerAddress(), this);
			if (!rootStream)
			{
				qCritical() << "Unknown protocol from" << socket->peerAddress().toString();
#ifdef QT_DEBUG
				qCritical() << "\tInitial line:" << line;
#endif
				socket->flush();
				socket->disconnectFromHost();
				return;
			}
			rootStream->recv(line);
		}
		
		// forward data to the stream
		while (socket->bytesAvailable())
		{
			rootStream->recv(socket->read(settings->value("bufferSize", 8192).toLongLong()));
			
			// Restart timer for read timeout, otherwise it would
			// expire during large file uploads.
			int readTimeout = settings->value("readTimeout", 10000).toInt();
			readTimer.start(readTimeout);
		}
	}
}
