/**
  @file
  @author Stefan Frings
*/

#include "httpsessionstore.h"

#include <QDateTime>
#include <QUuid>

using namespace qtwebapp;

HttpSessionStore::HttpSessionStore(const HttpSessionStoreConfig &cfg, QObject *parent)
  : QObject(parent)
  , cfg(cfg) {
	connect(&cleanupTimer, SIGNAL(timeout()), this, SLOT(sessionTimerEvent()));
	cleanupTimer.start(60000);
#ifdef CMAKE_DEBUG
	qDebug("HttpSessionStore: Sessions expire after %lli milliseconds", cfg.expirationTime);
#endif
}

HttpSessionStore::~HttpSessionStore() {
	cleanupTimer.stop();
}

QByteArray
HttpSessionStore::getSessionId(HttpRequest &request, HttpResponse &response) {
	// The session ID in the response has priority because this one will be used in the next request.
	mutex.lock();
	// Get the session ID from the response cookie
	QByteArray sessionId = response.getCookies().value(cfg.cookieName).getValue();
	if (sessionId.isEmpty()) {
		// Get the session ID from the request cookie
		sessionId = request.getCookie(cfg.cookieName);
	}
	// Clear the session ID if there is no such session in the storage.
	if (!sessionId.isEmpty()) {
		if (!sessions.contains(sessionId)) {
			qDebug("HttpSessionStore: received invalid session cookie with ID %s", sessionId.data());
			sessionId.clear();
		}
	}
	mutex.unlock();
	return sessionId;
}

HttpSession
HttpSessionStore::getSession(HttpRequest &request, HttpResponse &response, bool allowCreate) {
	QByteArray sessionId = getSessionId(request, response);
	mutex.lock();
	if (!sessionId.isEmpty()) {
		HttpSession session = sessions.value(sessionId);
		if (!session.isNull()) {
			mutex.unlock();
			// Refresh the session cookie
			response.setCookie(HttpCookie(cfg.cookieName,
			                              session.getId(),
			                              cfg.expirationTime / 1000,
			                              cfg.cookiePath,
			                              cfg.cookieComment,
			                              cfg.cookieDomain));
			session.setLastAccess();
			return session;
		}
	}
	// Need to create a new session
	if (allowCreate) {
		HttpSession session(true);
#ifdef CMAKE_DEBUG
		qDebug("HttpSessionStore: create new session with ID %s", session.getId().data());
#endif
		sessions.insert(session.getId(), session);
		response.setCookie(HttpCookie(cfg.cookieName,
		                              session.getId(),
		                              cfg.expirationTime / 1000,
		                              cfg.cookiePath,
		                              cfg.cookieComment,
		                              cfg.cookieDomain));
		mutex.unlock();
		return session;
	}
	// Return a null session
	mutex.unlock();
	return HttpSession();
}

HttpSession
HttpSessionStore::getSession(const QByteArray id) {
	mutex.lock();
	HttpSession session = sessions.value(id);
	mutex.unlock();
	session.setLastAccess();
	return session;
}

void
HttpSessionStore::sessionTimerEvent() {
	mutex.lock();
	qint64 now = QDateTime::currentMSecsSinceEpoch();
	QMap<QByteArray, HttpSession>::iterator i = sessions.begin();
	while (i != sessions.end()) {
		QMap<QByteArray, HttpSession>::iterator prev = i;
		++i;
		HttpSession session = prev.value();
		qint64 lastAccess = session.getLastAccess();
		if (now - lastAccess > cfg.expirationTime) {
			qDebug("HttpSessionStore: session %s expired", session.getId().data());
			emit sessionDeleted(session.getId());
			sessions.erase(prev);
		}
	}
	mutex.unlock();
}

/** Delete a session */
void
HttpSessionStore::removeSession(HttpSession session) {
	mutex.lock();
	emit sessionDeleted(session.getId());
	sessions.remove(session.getId());
	mutex.unlock();
}
