/**
  @file
  @author Stefan Frings
*/

#include "httplistener.h"
#include "httpconnectionhandler.h"
#include "httpconnectionhandlerpool.h"
#include <QCoreApplication>

using namespace qtwebapp;

HttpListener::HttpListener(const HttpServerConfig &cfg, HttpRequestHandler* requestHandler, QObject *parent)
    : QTcpServer(parent)
	, cfg(cfg)
{
    Q_ASSERT(requestHandler!=0);
    pool=NULL;
    this->requestHandler=requestHandler;
    // Reqister type of socketDescriptor for signal/slot handling
    qRegisterMetaType<tSocketDescriptor>("tSocketDescriptor");
    // Start listening
    listen();
}


HttpListener::~HttpListener()
{
    close();
#ifdef CMAKE_DEBUG
    qDebug("HttpListener: destroyed");
#endif
}


void HttpListener::listen()
{
    if (!pool)
    {
        pool=new HttpConnectionHandlerPool(cfg, requestHandler);
    }
    QTcpServer::listen(cfg.host, cfg.port);
    if (!isListening())
    {
        qCritical("HttpListener: Cannot bind on port %i: %s",cfg.port,qPrintable(errorString()));
    }
    else {
        qDebug("HttpListener: Listening on port %i", serverPort());
    }
}


void HttpListener::close() {
    QTcpServer::close();
#ifdef CMAKE_DEBUG
    qDebug("HttpListener: closed");
#endif
    if (pool) {
        delete pool;
        pool=NULL;
    }
}

void HttpListener::incomingConnection(tSocketDescriptor socketDescriptor) {
#ifdef SUPERVERBOSE
    qDebug("HttpListener: New connection");
#endif

    HttpConnectionHandler* freeHandler=NULL;
    if (pool)
    {
        freeHandler=pool->getConnectionHandler();
    }

    // Let the handler process the new connection.
    if (freeHandler)
    {
        // The descriptor is passed via event queue because the handler lives in another thread
        QMetaObject::invokeMethod(freeHandler, "handleConnection", Qt::QueuedConnection, Q_ARG(tSocketDescriptor, socketDescriptor));
    }
    else
    {
        // Reject the connection
        qWarning("HttpListener: Too many incoming connections");
        QTcpSocket* socket=new QTcpSocket(this);
        socket->setSocketDescriptor(socketDescriptor);
        connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
        socket->write("HTTP/1.1 503 too many connections\r\nConnection: close\r\n\r\nToo many connections\r\n");
        socket->disconnectFromHost();
    }
}
