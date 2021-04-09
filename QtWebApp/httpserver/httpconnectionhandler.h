/**
  @file
  @author Stefan Frings
*/

#pragma once

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpserverconfig.h"
#include "qtwebappglobal.h"

#include <QTcpSocket>
#include <QThread>
#include <QTimer>

#ifndef QT_NO_SSL
#include <QSslConfiguration>
#endif

namespace qtwebapp {

/** Alias for QSslConfiguration if OpenSSL is not supported */
#ifdef QT_NO_SSL
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
	class QTWEBAPP_EXPORT HttpConnectionHandler : public QObject {
		Q_OBJECT
		Q_DISABLE_COPY(HttpConnectionHandler)

	  public:
		/**
		  Constructor.
		  @param settings Configuration settings of the HTTP webserver
		  @param requestHandler Handler that will process each incoming HTTP request
		  @param sslConfiguration SSL (HTTPS) will be used if not NULL
		*/
		HttpConnectionHandler(const HttpServerConfig &cfg,
		                      HttpRequestHandler *requestHandler,
		                      const QSslConfiguration *sslConfiguration = nullptr);

		/** Destructor */
		virtual ~HttpConnectionHandler();

		/** Returns true, if this handler is in use. */
		bool isBusy();

		/** Mark this handler as busy */
		void setBusy();

	  private:
		/** Configuration */
		HttpServerConfig cfg;

		/** TCP socket of the current connection  */
		QTcpSocket *socket;

		/** The thread that processes events of this connection */ /** The thread that processes events of this connection */
		QThread *thread;

		/** Time for read timeout detection */
		QTimer readTimer;

		/** Storage for the current incoming HTTP request */
		HttpRequest *currentRequest;

		/** Dispatches received requests to services */
		HttpRequestHandler *requestHandler;

		/** This shows the busy-state from a very early time */
		bool busy;

		/** Configuration for SSL */
		const QSslConfiguration *sslConfiguration;

		/**  Create SSL or TCP socket */
		void createSocket();

	  public slots:

		/**
		  Received from from the listener, when the handler shall start processing a new connection.
		  @param socketDescriptor references the accepted connection.
		*/
		void handleConnection(qintptr socketDescriptor);

	  private slots:

		/** Received from the socket when a read-timeout occured */
		void readTimeout();

		/** Received from the socket when incoming data can be read */
		void read();

		/** Received from the socket when a connection has been closed */
		void disconnected();

		/** Cleanup after the thread is closed */
		void thread_done();
	};

} // end of namespace
