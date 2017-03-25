#include "httpserverconfig.h"

using namespace qtwebapp;

HttpServerConfig::HttpServerConfig()
{
}

HttpServerConfig::HttpServerConfig(const QSettings &settings)
{
	parseSettings(settings);
}

HttpServerConfig::HttpServerConfig(QSettings *settings)
{
	parseSettings(*settings);
}

void HttpServerConfig::parseSettings(const QSettings &settings)
{
#ifdef Q_OS_WIN
	if (settings.format() != QSettings::NativeFormat)
#endif
	fileName = settings.fileName();
	
	QString hoststr = settings.value("host").toString();
	host = hoststr.isEmpty() ? QHostAddress::Any : QHostAddress(hoststr);
	port = settings.value("port", port).toUInt();
	
	maxRequestSize = parseNum(settings.value("maxRequestSize", maxRequestSize), 1024);
	maxMultipartSize = parseNum(settings.value("maxMultipartSize", maxMultipartSize), 1024);
	
	cleanupInterval = parseNum(settings.value("cleanupInterval", cleanupInterval));
	
	minThreads = parseNum(settings.value("minThreads", minThreads));
	maxThreads = parseNum(settings.value("maxThreads", maxThreads));
	
	sslKeyFile = settings.value("sslKeyFile").toString();
	sslCertFile = settings.value("sslCertFile").toString();
}

// ###########################################################################################

HttpSessionStoreConfig::HttpSessionStoreConfig()
{
}

HttpSessionStoreConfig::HttpSessionStoreConfig(const QSettings &settings)
{
	parseSettings(settings);
}

HttpSessionStoreConfig::HttpSessionStoreConfig(QSettings *settings)
{
	parseSettings(*settings);
}

void HttpSessionStoreConfig::parseSettings(const QSettings &settings)
{
	expirationTime = parseNum(settings.value("expirationTime", (qulonglong)expirationTime), 1000);
	cookieName = settings.value("cookieName", cookieName).toByteArray();
	
	cookiePath = settings.value("cookiePath", cookiePath).toByteArray();
	cookieComment = settings.value("cookieComment", cookieComment).toByteArray();
	cookieDomain = settings.value("cookieDomain", cookieDomain).toByteArray();
}

// ###########################################################################################

StaticFileControllerConfig::StaticFileControllerConfig()
{
}

StaticFileControllerConfig::StaticFileControllerConfig(const QSettings &settings)
{
	parseSettings(settings);
}

StaticFileControllerConfig::StaticFileControllerConfig(QSettings *settings)
{
	parseSettings(*settings);
}

void StaticFileControllerConfig::parseSettings(const QSettings &settings)
{
#ifdef Q_OS_WIN
	if (settings.format() != QSettings::NativeFormat)
#endif
	fileName = settings.fileName();
	
	path = settings.value("path", path).toString();
	encoding = settings.value("encoding", encoding).toString();
	
	maxAge = parseNum(settings.value("maxAge", maxAge));
	maxCachedFileSize = parseNum(settings.value("maxCachedFileSize", maxCachedFileSize), 1024);
	
	cacheSize = parseNum(settings.value("cacheSize", cacheSize), 1024);
	cacheTime = parseNum(settings.value("cacheTime", cacheTime));
}
