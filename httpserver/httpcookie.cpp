/**
  @file
  @author Stefan Frings
*/

#include "httpcookie.h"

HttpCookie::HttpCookie()
{
	_version = 1;
	_maxAge = 0;
	_secure = false;
}

HttpCookie::HttpCookie(const QByteArray name, const QByteArray value, const int maxAge, const QByteArray path, const QByteArray comment, const QByteArray domain, const bool secure, const bool httponly)
	: _name(name)
	, _value(value)
	, _comment(comment)
	, _domain(domain)
	, _maxAge(maxAge)
	, _path(path)
	, _secure(secure)
	, _httpOnly(httponly)
	, _version(1)
{
}

HttpCookie::HttpCookie(const QByteArray source)
	: _maxAge(0)
	, _secure(false)
	, _httpOnly(false)
	, _version(1)
{
	QList<QByteArray> list = splitCSV(source);
	foreach (QByteArray part, list)
	{
	
		// Split the part into name and value
		QByteArray name;
		QByteArray value;
		int posi = part.indexOf('=');
		if (posi)
		{
			name = part.left(posi).trimmed();
			value = part.mid(posi + 1).trimmed();
		}
		else
		{
			name = part.trimmed();
			value = "";
		}
		
		// Set fields
		if (name == "Comment")
			_comment = value;
		else if (name == "Domain")
			_domain = value;
		else if (name == "Max-Age")
			_maxAge = value.toInt();
		else if (name == "Path")
			_path = value;
		else if (name == "Secure")
			_secure = true;
		else if (name == "HttpOnly")
			_httpOnly = true;
		else if (name == "Version")
			_version = value.toInt();
		else
		{
			if (this->_name.isEmpty())
			{
				this->_name = name;
				this->_value = value;
			}
			else
				qWarning("HttpCookie: Ignoring unknown %s=%s", name.data(), value.data());
		}
	}
}

QByteArray HttpCookie::toByteArray() const
{
	QByteArray buffer(_name);
	buffer.append('=');
	buffer.append(_value);
	if (!_comment.isEmpty())
	{
		buffer.append("; Comment=");
		buffer.append(_comment);
	}
	if (!_domain.isEmpty())
	{
		buffer.append("; Domain=");
		buffer.append(_domain);
	}
	if (_maxAge != 0)
	{
		buffer.append("; Max-Age=");
		buffer.append(QByteArray::number(_maxAge));
	}
	if (!_path.isEmpty())
	{
		buffer.append("; Path=");
		buffer.append(_path);
	}
	if (_secure)
		buffer.append("; Secure");
	if (_httpOnly)
		buffer.append("; HttpOnly");
	buffer.append("; Version=");
	buffer.append(QByteArray::number(_version));
	return buffer;
}

void HttpCookie::setName(const QByteArray name)
{
	this->_name = name;
}

void HttpCookie::setValue(const QByteArray value)
{
	this->_value = value;
}

void HttpCookie::setComment(const QByteArray comment)
{
	this->_comment = comment;
}

void HttpCookie::setDomain(const QByteArray domain)
{
	this->_domain = domain;
}

void HttpCookie::setMaxAge(const int maxAge)
{
	this->_maxAge = maxAge;
}

void HttpCookie::setPath(const QByteArray path)
{
	this->_path = path;
}

void HttpCookie::setSecure(const bool secure)
{
	this->_secure = secure;
}

void HttpCookie::setHttpOnly(const bool httponly)
{
	this->_httpOnly = httponly;
}

QByteArray HttpCookie::name() const
{
	return _name;
}

QByteArray HttpCookie::value() const
{
	return _value;
}

QByteArray HttpCookie::comment() const
{
	return _comment;
}

QByteArray HttpCookie::domain() const
{
	return _domain;
}

int HttpCookie::maxAge() const
{
	return _maxAge;
}

QByteArray HttpCookie::path() const
{
	return _path;
}

bool HttpCookie::secure() const
{
	return _secure;
}

bool HttpCookie::httpOnly() const
{
	return _httpOnly;
}

int HttpCookie::version() const
{
	return _version;
}

QList<QByteArray> HttpCookie::splitCSV(const QByteArray source)
{
	bool inString = false;
	QList<QByteArray> list;
	QByteArray buffer;
	for (int i = 0; i < source.size(); ++i)
	{
		char c = source.at(i);
		if (inString == false)
		{
			if (c == '\"')
				inString = true;
			else if (c == ';')
			{
				QByteArray trimmed = buffer.trimmed();
				if (!trimmed.isEmpty())
					list.append(trimmed);
				buffer.clear();
			}
			else
				buffer.append(c);
		}
		else
		{
			if (c == '\"')
				inString = false;
			else
				buffer.append(c);
		}
	}
	QByteArray trimmed = buffer.trimmed();
	if (!trimmed.isEmpty())
		list.append(trimmed);
	return list;
}
