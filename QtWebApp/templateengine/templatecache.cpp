#include "templatecache.h"

#include <QDateTime>
#include <QSet>
#include <QStringList>

using namespace qtwebapp;

TemplateCache::TemplateCache(const TemplateEngineConfig &cfg, QObject *parent) : TemplateLoader(cfg, parent) {
	cache.setMaxCost(cfg.cacheSize);
	cacheTimeout = cfg.cacheTime;
#ifdef CMAKE_DEBUG
	qDebug("TemplateCache: timeout=%i, size=%i", cacheTimeout, cache.maxCost());
#endif
}

QString TemplateCache::tryFile(const QString &localizedName) {
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	mutex.lock();
	// search in cache
#ifdef CMAKE_DEBUG
	qDebug("TemplateCache: trying cached %s", qPrintable(localizedName));
#endif
	CacheEntry *entry = cache.object(localizedName);
	if (entry && (cacheTimeout == 0 || entry->created > now - cacheTimeout)) {
		mutex.unlock();
		return entry->document;
	}
	// search on filesystem
	entry = new CacheEntry();
	entry->created = now;
	entry->document = TemplateLoader::tryFile(localizedName);
	// Store in cache even when the file did not exist, to remember that there is no such file
	cache.insert(localizedName, entry, entry->document.size());
	mutex.unlock();
	return entry->document;
}
