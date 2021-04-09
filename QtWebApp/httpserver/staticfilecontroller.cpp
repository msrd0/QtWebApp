/**
  @file
  @author Stefan Frings
*/

#include "staticfilecontroller.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

using namespace qtwebapp;

StaticFileController::StaticFileController(const StaticFileControllerConfig &cfg, QObject* parent)
	:HttpRequestHandler(parent)
{
	maxAge=cfg.maxAge;
	encoding=cfg.encoding;
	docroot=cfg.path;
	if(!cfg.fileName.isEmpty() && !(docroot.startsWith(":/") || !docroot.startsWith("qrc://")))
	{
		// Convert relative path to absolute, based on the directory of the config file.
		if (QDir::isRelativePath(docroot))
		{
			QFileInfo configFile(cfg.fileName);
			docroot=QFileInfo(configFile.absolutePath(),docroot).absoluteFilePath();
		}
	}
#ifdef CMAKE_DEBUG
	qDebug("StaticFileController: docroot=%s, encoding=%s, maxAge=%i",qPrintable(docroot),qPrintable(encoding),maxAge);
#endif
	maxCachedFileSize=cfg.maxCachedFileSize;
	cache.setMaxCost(cfg.cacheSize);
	cacheTimeout=cfg.cacheTime;
	long int cacheMaxCost = (long int)cache.maxCost();
#ifdef CMAKE_DEBUG
	qDebug("StaticFileController: cache timeout=%i, size=%li", cacheTimeout, cacheMaxCost);
#endif
}


void StaticFileController::service(HttpRequest &request, HttpResponse &response)
{
	QByteArray path=request.getPath();
	// Check whether the browsers cache is up to date
	if (!request.getHeader("If-None-Match").isEmpty() &&
			request.getHeader("If-None-Match") == ("\"" + etag.value(path) + "\""))
	{
		response.setStatus(304, "Not Modified");
		return;
	}
	// Check if we have the file in cache
	qint64 now=QDateTime::currentMSecsSinceEpoch();
	mutex.lock();
	CacheEntry* entry=cache.object(path);
	if (entry && (cacheTimeout==0 || entry->created>now-cacheTimeout))
	{
		QByteArray document=entry->document; //copy the cached document, because other threads may destroy the cached entry immediately after mutex unlock.
		QByteArray filename=entry->filename;
		response.setHeader("ETag", "\"" + etag.value(path) + "\"");
		mutex.unlock();
#ifdef CMAKE_DEBUG
		qDebug("StaticFileController: Cache hit for %s",path.data());
#endif
		setContentType(filename,response);
		response.setHeader("Cache-Control","max-age="+QByteArray::number(maxAge/1000));
		response.write(document);
	}
	else
	{
		mutex.unlock();
		// The file is not in cache.
#ifdef CMAKE_DEBUG
		qDebug("StaticFileController: Cache miss for %s",path.data());
#endif
		// Forbid access to files outside the docroot directory
		if (path.contains("/.."))
		{
			qWarning("StaticFileController: detected forbidden characters in path %s",path.data());
			response.setStatus(403,"forbidden");
			response.write("403 forbidden",true);
			return;
		}
		// If the filename is a directory, append index.html.
		if (QFileInfo(docroot+path).isDir())
		{
			path+="/index.html";
		}
		// Try to open the file
		QFile file(docroot+path);
#ifdef CMAKE_DEBUG
		qDebug("StaticFileController: Open file %s",qPrintable(file.fileName()));
#endif
		if (file.open(QIODevice::ReadOnly))
		{
			setContentType(path,response);
			response.setHeader("Cache-Control","max-age="+QByteArray::number(maxAge/1000));
			if (file.size()<=maxCachedFileSize)
			{
				// Return the file content and store it also in the cache
				entry=new CacheEntry();
				while (!file.atEnd() && !file.error())
				{
					QByteArray buffer=file.read(65536);
					entry->document.append(buffer);
				}
				entry->created=now;
				entry->filename=path;
				mutex.lock();
				etag.insert(path, QCryptographicHash::hash(entry->document, QCryptographicHash::Md5).toHex());
				response.setHeader("ETag", "\"" + etag.value(path) + "\"");
				response.write(entry->document);
				cache.insert(request.getPath(),entry,entry->document.size());
				mutex.unlock();
			}
			else
			{
				// Return the file content, do not store in cache
				while (!file.atEnd() && !file.error())
				{
					response.write(file.read(65536));
				}
			}
			file.close();
		}
		else {
			if (file.exists())
			{
				qWarning("StaticFileController: Cannot open existing file %s for reading",qPrintable(file.fileName()));
				response.setStatus(403,"forbidden");
				response.write("403 forbidden",true);
			}
			else
			{
				response.setStatus(404,"not found");
				response.write("404 not found",true);
			}
		}
	}
}

void StaticFileController::setContentType(const QString &fileName, HttpResponse &response) const
{
	if (fileName.endsWith(".png"))
	{
		response.setHeader("Content-Type", "image/png");
	}
	else if (fileName.endsWith(".jpg"))
	{
		response.setHeader("Content-Type", "image/jpeg");
	}
	else if (fileName.endsWith(".gif"))
	{
		response.setHeader("Content-Type", "image/gif");
	}
	else if (fileName.endsWith(".pdf"))
	{
		response.setHeader("Content-Type", "application/pdf");
	}
	else if (fileName.endsWith(".txt"))
	{
		response.setHeader("Content-Type", qPrintable("text/plain; charset="+encoding));
	}
	else if (fileName.endsWith(".html") || fileName.endsWith(".htm"))
	{
		response.setHeader("Content-Type", qPrintable("text/html; charset="+encoding));
	}
	else if (fileName.endsWith(".css"))
	{
		response.setHeader("Content-Type", "text/css");
	}
	else if (fileName.endsWith(".js"))
	{
		response.setHeader("Content-Type", "text/javascript");
	}
	else if (fileName.endsWith(".svg"))
	{
		response.setHeader("Content-Type", "image/svg+xml");
	}
	else if (fileName.endsWith(".woff"))
	{
		response.setHeader("Content-Type", "font/woff");
	}
	else if (fileName.endsWith(".woff2"))
	{
		response.setHeader("Content-Type", "font/woff2");
	}
	else if (fileName.endsWith(".ttf"))
	{
		response.setHeader("Content-Type", "application/x-font-ttf");
	}
	else if (fileName.endsWith(".eot"))
	{
		response.setHeader("Content-Type", "application/vnd.ms-fontobject");
	}
	else if (fileName.endsWith(".otf"))
	{
		response.setHeader("Content-Type", "application/font-otf");
    }
    else if (fileName.endsWith(".json"))
    {
        response.setHeader("Content-Type", "application/json");
    }
    else if (fileName.endsWith(".xml"))
    {
        response.setHeader("Content-Type", "text/xml");
	}
	// Todo: add all of your content types
	else
	{
		qDebug("StaticFileController: unknown MIME type for filename '%s'", qPrintable(fileName));
		response.setHeader("Content-Type", "application/octet-stream");
	}
}
