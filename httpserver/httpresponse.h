/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "httpcookie.h"
#include "httpglobal.h"
#include "httprequest.h"
#include "httpresponsestatus.h"
#include <QMap>
#include <QString>

class HttpStream;

/**
  This object represents a HTTP response, in particular the response headers.
  <p>
  Example code for proper response generation:
  <code><pre>
    response.setStatus(200,"OK"); // optional, because this is the default
    response.writeBody("Hello");
    response.writeBody("World!",true);
  </pre></code>
  <p>
  Example how to return an error:
  <code><pre>
    response.setStatus(500,"server error");
    response.write("The request cannot be processed because the servers is broken",true);
  </pre></code>
  <p>
  For performance reason, writing a single or few large packets is better than writing
  many small packets. In case of large responses (e.g. file downloads), a Content-Length
  header should be set before calling write(). Web Browsers use that information to display
  a progress bar.
*/

class DECLSPEC HttpResponse
{
	Q_DISABLE_COPY(HttpResponse)
	
public:
	
	HttpResponse(HttpStream *stream);
	~HttpResponse();
	
	/**
	  Set a HTTP response header
	  @param name name of the header
	  @param value value of the header
	*/
	void setHeader(QByteArray name, QByteArray value);
	
	/**
	  Set a HTTP response header
	  @param name name of the header
	  @param value value of the header
	*/
	void setHeader(QByteArray name, int value);
	
	/** Get the map of HTTP response headers */
	QMap<QByteArray, QByteArray> &headers();
	
	/** Get the map of cookies */
	QMap<QByteArray, HttpCookie> &cookies();
	
	/** Set status code. The default is 200 OK. */
	void setStatus(HttpResponseStatus status);
	
	/** Returns the status code. */
	HttpResponseStatus status() const { return _status; }
	
	/**
	  Write body data to the socket.
	  <p>
	  The HTTP status line and headers are sent automatically before the first
	  byte of the body gets sent.
	  <p>
	  If the response contains only a single chunk (indicated by lastPart=true),
	  the response is transferred in traditional mode with a Content-Length
	  header, which is automatically added if not already set before.
	  <p>
	  Otherwise, each part is transferred in chunked mode.
	  @param data Data bytes of the body
	  @param lastPart Indicator, if this is the last part of the response.
	*/
	void write(QByteArray data, bool lastPart = false);
	
	/**
	  Indicates wheter the body has been sent completely. Used by the connection
	  handler to terminate the body automatically when necessary.
	*/
	bool hasSentLastPart() const;
	
	/**
	  Set a cookie. Cookies are sent together with the headers when the first
	  call to write() occurs.
	*/
	void setCookie(const HttpCookie &cookie);
	
	/**
	  Send a redirect response to the browser.
	  @param url Destination URL
	*/
	void redirect(const QByteArray &url);
	
private:
	/** Request headers */
	QMap<QByteArray, QByteArray> _headers;
	
	/** The Stream to write tho output to. */
	HttpStream *_stream;
	
	/** HTTP status code. */
	HttpResponseStatus _status;
	
	/** Indicator whether headers have been sent */
	bool _sentHeaders;
	
	/** Indicator whether the body has been sent completely */
	bool _sentLastPart;
	
	/** Cookies */
	QMap<QByteArray, HttpCookie> _cookies;
	
};

#endif // HTTPRESPONSE_H
