/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPCONNECTIONHANDLER_H
#define HTTPCONNECTIONHANDLER_H

#ifndef QT_NO_OPENSSL
#include <QSslConfiguration>
#endif
#include "httpglobal.h"
#include "httprequest.h"
#include "httprequesthandler.h"
#include <QSettings>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>

class HttpStream;

/** Alias type definition, for compatibility to different Qt versions */
#if QT_VERSION >= 0x050000
typedef qintptr tSocketDescriptor;
#else
typedef int tSocketDescriptor;
#endif

/** Alias for QSslConfiguration if OpenSSL is not supported */
#ifdef QT_NO_OPENSSL
#define QSslConfiguration QObject
#endif

/**
  The connection handler accepts incoming connections and dispatches incoming requests to to a
  request mapper. Since HTTP clients can send multiple requests before waiting for the response,
  the incoming requests are queued and processed one after the other.
  <p>
  Example for the required configuration settings:
  <code><pre>
  readTimeout=60000
  maxRequestSize=16000
  maxMultiPartSize=1000000
  </pre></code>
  <p>
  The readTimeout value defines the maximum time to wait for a complete HTTP request.
  @see HttpRequest for description of config settings maxRequestSize and maxMultiPartSize.
*/
class DECLSPEC HttpConnectionHandler : public QThread
{
	Q_OBJECT
	Q_DISABLE_COPY(HttpConnectionHandler)
	
public:
	/**
	  Constructor.
	  @param settings Configuration settings of the HTTP webserver
	  @param requestHandler Handler that will process each incoming HTTP request
	  @param sslConfiguration SSL (HTTPS) will be used if not NULL
	*/
	HttpConnectionHandler(QSettings *settings, HttpRequestHandler *_requestHandler, QSslConfiguration *sslConfiguration = NULL);
	
	/** Destructor */
	virtual ~HttpConnectionHandler();
	
	/** Returns true, if this handler is in use. */
	bool isBusy();
	
	/** Mark this handler as busy */
	void setBusy();
	
	/** Called from the stream when the protocol should be changed. Using Protocol::UNKNOWN
	 * indicates that the connection should be closed because of malformed data received
	 * from the client. */
	void changeProtocol(HttpRequest::Protocol protocol);
	
	/** Called from the stream when there is data to be written. */
	void send(const QByteArray &data);
	
	/** Returns the RequestHandler associated to this ConnectionHandler. */
	HttpRequestHandler *requestHandler() { return _requestHandler; }
	
public slots:
	/**
	  Received from from the listener, when the handler shall start processing a new connection.
	  @param socketDescriptor references the accepted connection.
	*/
	void handleConnection(tSocketDescriptor socketDescriptor);
	
private slots:
	/** Received from the socket when a read-timeout occured */
	void readTimeout();
	
	/** Received from the socket when incoming data can be read */
	void read();
	
	/** Received from the socket when a connection has been closed */
	void disconnected();
	
private:
	/** Executes the threads own event loop */
	void run();
	
	/**  Create SSL or TCP socket */
	void createSocket();
	
	/** Configuration settings */
	QSettings *settings;
	
	/** TCP socket of the current connection  */
	QTcpSocket *socket;
	
	/** Time for read timeout detection */
	QTimer readTimer;
	
	/** The Protocol used by this connection. */
	HttpRequest::Protocol protocol;
	
	/** Storage for the root stream. */
	HttpStream *rootStream;
	
	/** Dispatches received requests to services */
	HttpRequestHandler *_requestHandler;
	
	/** This shows the busy-state from a very early time */
	bool busy;
	
	/** Configuration for SSL */
	QSslConfiguration *sslConfiguration;
	
};

#endif // HTTPCONNECTIONHANDLER_H
