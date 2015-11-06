/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "httpglobal.h"
#include <QByteArray>
#include <QHostAddress>
#include <QMap>
#include <QMultiMap>
#include <QTemporaryFile>
#include <QUuid>

/**
 * This object represents a single HTTP request. It is created by the HttpStream
 * and passed to the HttpRequestHandler.
 */

class DECLSPEC HttpRequest
{
	Q_DISABLE_COPY(HttpRequest)
	friend class HttpSessionStore;
	
public:
	/** All supported protocols. */
	enum Protocol
	{
		/** Unknown */
		UNKNOWN = 0,
		/** HTTP/1.0 */
		HTTP_1_0 = 100,
		/** HTTP/1.1 */
		HTTP_1_1 = 101,
		/** HTTP/2.0 */
		HTTP_2_0 = 200
	};
	
	/** All HTTP Methods. */
	enum Method
	{
		GET,
		POST,
		HEAD,
		OPTIONS,
		PUT,
		DELETE,
		TRACE,
		CONNECT
	};
	
	HttpRequest(Protocol protocol, const QHostAddress &address);
	virtual ~HttpRequest();
	
	/** Get the method of the HTTP request  (e.g. HttpRequest::GET) */
	Method method() const { return _method; }
	/** Get the method of the HTTP request as a string (e.g. "GET"). */
	QString methodStr() const;
	
	/** Set the method of the HTTP request. */
	void setMethod(Method method) { _method = method; }
	
	/** Get the decoded path of the HTTP request (e.g. "/index.html") */
	QString path() const { return _path; }
	
	/** Set the decoded path of the HTTP request. */
	void setPath(const QString &path) { _path = path; }
	
	/** Get the protocol. */
	Protocol protocol() const { return _protocol; }
	/** Get the string representation of the protocol (e.g. "HTTP\2.0"). */
	QString protocolStr() const;
	
	/** Get the address of the client. */
	QHostAddress address() const { return _address; }
	
	/**
	  Get the value of a HTTP request header.
	  @param name Name of the header
	  @return If the header occurs multiple times, only the last
	  one is returned.
	*/
	QByteArray getHeader(const QByteArray &name) const;
	
	/**
	  Get the values of a HTTP request header.
	  @param name Name of the header
	*/
	QList<QByteArray> getHeaders(const QByteArray &name) const;
	
	/** Get all HTTP request headers */
	QMultiMap<QByteArray, QByteArray> getHeaderMap() const;
	
	/** Inserts the request header. */
	void insertHeader(const QByteArray &name, const QByteArray &value);
	
	/**
	  Get the value of a HTTP request parameter.
	  @param name Name of the parameter
	  @return If the parameter occurs multiple times, only the last
	  one is returned.
	*/
	QByteArray getParameter(const QByteArray &name) const;
	
	/**
	  Get the values of a HTTP request parameter.
	  @param name Name of the parameter
	*/
	QList<QByteArray> getParameters(const QByteArray &name) const;
	
	/** Get all HTTP request parameters */
	QMultiMap<QByteArray, QByteArray> getParameterMap() const;
	
	/** Get the HTTP request body  */
	QByteArray getBody() const;
	
	/**
	  Decode an URL parameter.
	  E.g. replace "%23" by '#' and replace '+' by ' '.
	  @param source The url encoded strings
	  @see QUrl::toPercentEncoding for the reverse direction
	*/
	static QByteArray urlDecode(const QByteArray source);
	
	/**
	  Get an uploaded file. The file is already open. It will
	  be closed and deleted by the destructor of this HttpRequest
	  object (after processing the request).
	  <p>
	  For uploaded files, the method getParameters() returns
	  the original fileName as provided by the calling web browser.
	*/
	QTemporaryFile *getUploadedFile(const QByteArray fieldName);
	
	/**
	  Get the value of a cookie
	  @param name Name of the cookie
	*/
	QByteArray getCookie(const QByteArray &name) const;
	
	/** Get the map of cookies */
	QMap<QByteArray, QByteArray> &getCookieMap();
	
private:

	/** Request headers */
	QMultiMap<QByteArray, QByteArray> _headers;
	
	/** Parameters of the request */
	QMultiMap<QByteArray, QByteArray> _parameters;
	
	/** Uploaded files of the request, key is the field name. */
	QMap<QByteArray, QTemporaryFile*> _uploadedFiles;
	
	/** Received cookies */
	QMap<QByteArray, QByteArray> _cookies;
	
	/** Storage for raw body data */
	QByteArray _bodyData;
	
	/** Request method */
	Method _method;
	
	/** Request path (in raw encoded format) */
	QString _path;
	
	/** Request protocol. */
	Protocol _protocol;
	
	/** The address of the peer. */
	QHostAddress _address;
	
};

#endif // HTTPREQUEST_H
