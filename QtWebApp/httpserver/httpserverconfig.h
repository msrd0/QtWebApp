#pragma once

#include "qtwebappglobal.h"

#include <QHostAddress>
#include <QSettings>

namespace qtwebapp {
class HttpConnectionHandlerPool;
class StaticFileController;

/**
 * This class stores all configuration information for the `HttpListener`
 * class. It can be either created as a standard object and filled from
 * c++ code, or be constructed from a `QSettings` object. See `HttpListener`
 * for an example of such a configuration file.
 */
class QTWEBAPP_EXPORT HttpServerConfig
{
	friend class HttpConnectionHandlerPool;
	
public:
	/** Creates a config with all standard values. */
	HttpServerConfig();
	/** Reads the configuration from the `QSettings` object. */
	HttpServerConfig(const QSettings &settings);
	/** Reads the configuration from the `QSettings` object. */
	HttpServerConfig(QSettings *settings);
	
	/// The address for the server to listen on.
	QHostAddress host = QHostAddress::Any;
	/// The port for the server to listen on.
	quint16 port = 0;
	
	/// The maximum size of an HTTP request.
	int maxRequestSize = 16e3;
	/// The maximum size of a body of a multipart/form-data request.
	int maxMultipartSize = 1e6;
	
	/// The maximum amount of time to wait for an HTTP request to complete.
	int readTimeout = 1e4;
	
	/// The interval to search for idle connection handlers and kill them.
	int cleanupInterval = 1e3;
	/// The minimum of idle connection handlers to keep.
	int minThreads = 1;
	/// The maximum amount of connection handlers.
	int maxThreads = 100;
	
	/// The file required for SSL support.
	QString sslKeyFile, sslCertFile;
	
private:
	void parseSettings(const QSettings &settings);
	
	/// The filename of the settings if read from a file. It is used to resolve
	/// relative paths.
	QString fileName;
};

/**
 * This class stores all configuration information for the `HttpSessionStore`
 * class. It can be either created as a standard object and filled from
 * c++ code, or be constructed from a `QSettings` object. See `HttpSessionStore`
 * for an example of such a configuration file.
 */
class QTWEBAPP_EXPORT HttpSessionStoreConfig
{
public:
	/** Creates a config with all standard values. */
	HttpSessionStoreConfig();
	/** Reads the configuration from the `QSettings` object. */
	HttpSessionStoreConfig(const QSettings &settings);
	/** Reads the configuration frem the `QSettings` pointer. */
	HttpSessionStoreConfig(QSettings *settings);
	
	/// The expiration time of the cookie.
	quint64 expirationTime = 3600e3;
	/// The name of the cookie.
	QByteArray cookieName = "sessionid";
	
	/// The url path where the session is valid. This is usefull when you have
	/// data not related to the session in `/static/` and session related data in
	/// `/content/` or similar.
	QByteArray cookiePath = "/";
	/// The comment of the cookie.
	QByteArray cookieComment;
	/// The domain of the cookie.
	QByteArray cookieDomain;
	
private:
	void parseSettings(const QSettings &settings);
};

/**
 * This class stores all configuration information for the `StaticFileController`
 * class. It can be either created as a astandard object and filled from c++
 * code, or be constructed from a `QSettings` object. See `StaticFileController`
 * for an example of such a configuration file.
 */
class QTWEBAPP_EXPORT StaticFileControllerConfig
{
	friend class StaticFileController;
	
public:
	/** Creates a config with all standard values. */
	StaticFileControllerConfig();
	/** Reads the configuration from the `QSettings` object. */
	StaticFileControllerConfig(const QSettings &settings);
	/** Reads the configuration from the `QSettings` object. */
	StaticFileControllerConfig(QSettings *settings);
	
	/// The path where the static files can be found. This can be either an
	/// absolute or relativ path or an qt resource path.
	QString path = ".";
	
	/// The encoding that is sent to the web browser in case of text files.
	QString encoding = "UTF-8";
	
	/// The amount of time the file should reside in the browsers cache.
	int maxAge = 6e4;
	
	/// The maximum size of a file to get cached.
	int maxCachedFileSize = 2<<15;
	
	/// The size of the server cache.
	int cacheSize = 1e6;
	/// The timeout of each file in the servers cache.
	int cacheTime = 6e4;
	
private:
	void parseSettings(const QSettings &settings);
	
	/// The filename of the settings if read from a file. It is used to resolve
	/// relative paths.
	QString fileName;
};

}
