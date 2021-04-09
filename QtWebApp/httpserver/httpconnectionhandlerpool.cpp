#include "httpconnectionhandlerpool.h"

#include "httplistener.h"

#include <QDir>

#ifndef QT_NO_OPENSSL
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#endif

using namespace qtwebapp;

HttpConnectionHandlerPool::HttpConnectionHandlerPool(const HttpServerConfig &cfg, HttpRequestHandler *requestHandler)
    : QObject(), cfg(cfg), requestHandler(requestHandler) {
	sslConfiguration = nullptr;
	loadSslConfig();
	cleanupTimer.start(cfg.cleanupInterval);
	connect(&cleanupTimer, SIGNAL(timeout()), SLOT(cleanup()));
}

HttpConnectionHandlerPool::~HttpConnectionHandlerPool() {
	// delete all connection handlers and wait until their threads are closed
	foreach (HttpConnectionHandler *handler, pool) { delete handler; }
	delete sslConfiguration;
#ifdef CMAKE_DEBUG
	qDebug("HttpConnectionHandlerPool (%p): destroyed", this);
#endif
}

HttpConnectionHandler *HttpConnectionHandlerPool::getConnectionHandler() {
	HttpConnectionHandler *freeHandler = nullptr;
	mutex.lock();
	// find a free handler in pool
	foreach (HttpConnectionHandler *handler, pool) {
		if (!handler->isBusy()) {
			freeHandler = handler;
			freeHandler->setBusy();
			break;
		}
	}
	// create a new handler, if necessary
	if (!freeHandler) {
		int maxConnectionHandlers = cfg.maxThreads;
		if (pool.count() < maxConnectionHandlers) {
			freeHandler = new HttpConnectionHandler(cfg, requestHandler, sslConfiguration);
			freeHandler->setBusy();
			pool.append(freeHandler);
		}
	}
	mutex.unlock();
	return freeHandler;
}

void HttpConnectionHandlerPool::cleanup() {
	int maxIdleHandlers = cfg.minThreads;
	int idleCounter = 0;
	mutex.lock();
	foreach (HttpConnectionHandler *handler, pool) {
		if (!handler->isBusy()) {
			if (++idleCounter > maxIdleHandlers) {
				delete handler;
				pool.removeOne(handler);
#ifdef CMAKE_DEBUG
				qDebug("HttpConnectionHandlerPool: Removed connection handler (%p), pool size is now %i", handler,
				       pool.size());
#endif
				break; // remove only one handler in each interval
			}
		}
	}
	mutex.unlock();
}

void HttpConnectionHandlerPool::loadSslConfig() {
	// If certificate and key files are configured, then load them
	QString sslKeyFileName = cfg.sslKeyFile;
	QString sslCertFileName = cfg.sslCertFile;
	if (!sslKeyFileName.isEmpty() && !sslCertFileName.isEmpty()) {
#ifdef QT_NO_OPENSSL
		qWarning("HttpConnectionHandlerPool: SSL is not supported");
#else
		// Convert relative fileNames to absolute, based on the directory of the config file.
		if (!cfg.fileName.isEmpty()) {
			QFileInfo configFile(cfg.fileName);
			if (QDir::isRelativePath(sslKeyFileName)) {
				sslKeyFileName = QFileInfo(configFile.absolutePath(), sslKeyFileName).absoluteFilePath();
			}
			if (QDir::isRelativePath(sslCertFileName)) {
				sslCertFileName = QFileInfo(configFile.absolutePath(), sslCertFileName).absoluteFilePath();
			}
		}

		// Load the SSL certificate
		QFile certFile(sslCertFileName);
		if (!certFile.open(QIODevice::ReadOnly)) {
			qCritical("HttpConnectionHandlerPool: cannot open sslCertFile %s", qPrintable(sslCertFileName));
			return;
		}
		QSslCertificate certificate(&certFile, QSsl::Pem);
		certFile.close();

		// Load the key file
		QFile keyFile(sslKeyFileName);
		if (!keyFile.open(QIODevice::ReadOnly)) {
			qCritical("HttpConnectionHandlerPool: cannot open sslKeyFile %s", qPrintable(sslKeyFileName));
			return;
		}
		QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
		keyFile.close();

		// Create the SSL configuration
		sslConfiguration = new QSslConfiguration();
		sslConfiguration->setLocalCertificate(certificate);
		sslConfiguration->setPrivateKey(sslKey);
		sslConfiguration->setPeerVerifyMode(QSslSocket::VerifyNone);
		sslConfiguration->setProtocol(QSsl::TlsV1SslV3);

#ifdef CMAKE_DEBUG
		qDebug("HttpConnectionHandlerPool: SSL settings loaded");
#endif
#endif // QT_NO_OPENSSL
	}
}
