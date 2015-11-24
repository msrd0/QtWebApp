/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPCOOKIE_H
#define HTTPCOOKIE_H

#include <QList>
#include <QByteArray>
#include "httpglobal.h"

/**
  HTTP cookie as defined in RFC 2109. This class can also parse
  RFC 2965 cookies, but skips fields that are not defined in RFC
  2109.
*/

class DECLSPEC HttpCookie
{
public:

	/** Creates an empty cookie */
	HttpCookie();
	
	/**
	  Create a cookie and set name/value pair.
	  @param name name of the cookie
	  @param value value of the cookie
	  @param maxAge maximum age of the cookie in seconds. 0=discard immediately
	  @param path Path for that the cookie will be sent, default="/" which means the whole domain
	  @param comment Optional comment, may be displayed by the web browser somewhere
	  @param domain Optional domain for that the cookie will be sent. Defaults to the current domain
	  @param secure If true, the cookie will only be sent on secure connections
	*/
	HttpCookie(const QByteArray _name, const QByteArray _value, const int _maxAge, const QByteArray _path = "/", const QByteArray _comment = QByteArray(), const QByteArray _domain = QByteArray(), const bool _secure = false, const bool httponly = false);
	
	/**
	  Create a cookie from a string.
	  @param source String as received in a HTTP Cookie2 header.
	*/
	HttpCookie(const QByteArray source);
	
	/** Convert this cookie to a string that may be used in a Set-Cookie header. */
	QByteArray toByteArray() const ;
	
	/**
	  Split a string list into parts, where each part is delimited by semicolon.
	  Semicolons within double quotes are skipped. Double quotes are removed.
	*/
	static QList<QByteArray> splitCSV(const QByteArray source);
	
	/** Set the name of this cookie */
	void setName(const QByteArray _name);
	
	/** Set the value of this cookie */
	void setValue(const QByteArray _value);
	
	/** Set the comment of this cookie */
	void setComment(const QByteArray _comment);
	
	/** Set the domain of this cookie */
	void setDomain(const QByteArray _domain);
	
	/** Set the maximum age of this cookie in seconds. 0=discard immediately */
	void setMaxAge(const int _maxAge);
	
	/** Set the path for that the cookie will be sent, default="/" which means the whole domain */
	void setPath(const QByteArray _path);
	
	/** Set secure mode, so that the cookie will only be sent on secure connections */
	void setSecure(const bool _secure);
	
	/** Set httponly mode, so that the cookie can only be access from javascript. */
	void setHttpOnly(const bool httponly);
	
	/** Get the name of this cookie */
	QByteArray name() const;
	
	/** Get the value of this cookie */
	QByteArray value() const;
	
	/** Get the comment of this cookie */
	QByteArray comment() const;
	
	/** Get the domain of this cookie */
	QByteArray domain() const;
	
	/** Set the maximum age of this cookie in seconds. */
	int maxAge() const;
	
	/** Set the path of this cookie */
	QByteArray path() const;
	
	/** Get the secure flag of this cookie */
	bool secure() const;
	
	/** Get the httponly flag of this cookie */
	bool httpOnly() const;
	
	/** Returns always 1 */
	int version() const;
	
private:

	QByteArray _name;
	QByteArray _value;
	QByteArray _comment;
	QByteArray _domain;
	int _maxAge;
	QByteArray _path;
	bool _secure;
	bool _httpOnly;
	int _version;
	
};

#endif // HTTPCOOKIE_H
