/**
  @file
  @author Stefan Frings
*/

#include "httpcookie.h"
#include "httprequest.h"

#include <QDir>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QRegularExpression>
#include <QTextCodec>

HttpRequest::HttpRequest(Protocol protocol, const QHostAddress &address)
	: _protocol(protocol)
	, _address(address)
{
	Q_ASSERT(protocol != UNKNOWN);
}

HttpRequest::~HttpRequest()
{
	foreach (QByteArray key, _uploadedFiles.keys())
	{
		QTemporaryFile *file = _uploadedFiles.value(key);
		file->close();
		delete file;
	}
}

QString HttpRequest::methodStr() const
{
	switch (method())
	{
	case GET:
		return "GET";
	case POST:
		return "POST";
	case HEAD:
		return "HEAD";
	case OPTIONS:
		return "OPTIONS";
	case PUT:
		return "PUT";
	case DELETE:
		return "DELETE";
	case TRACE:
		return "TRACE";
	case CONNECT:
		return "CONNECT";
	default:
		return QString();
	}
}

QString HttpRequest::protocolStr() const
{
	switch (protocol())
	{
	case HTTP_1_0:
		return "HTTP/1.0";
	case HTTP_1_1:
		return "HTTP/1.1";
	case HTTP_2_0:
		return "HTTP/2.0";
	default:
		return QString();
	}
}

QByteArray HttpRequest::header(const QByteArray &name) const
{
	return _headers.value(name.toLower());
}

QList<QByteArray> HttpRequest::headers(const QByteArray &name) const
{
	return _headers.values(name.toLower());
}

QMultiMap<QByteArray, QByteArray> HttpRequest::headerMap() const
{
	return _headers;
}

void HttpRequest::insertHeader(const QByteArray &name, const QByteArray &value)
{
	if (name.toLower() == "cookie")
	{
		for (QByteArray val : value.split(';'))
		{
			HttpCookie cookie(val);
			_cookies.insert(cookie.name(), cookie.value());
		}
	}
	else
		_headers.insert(name.toLower(), value);
}

QByteArray HttpRequest::parameter(const QByteArray &name) const
{
	return _parameters.value(name);
}

QList<QByteArray> HttpRequest::parameters(const QByteArray &name) const
{
	return _parameters.values(name);
}

QMultiMap<QByteArray, QByteArray> HttpRequest::parameterMap() const
{
	return _parameters;
}

QByteArray HttpRequest::body() const
{
	return _bodyData;
}

void HttpRequest::appendBody(const QByteArray &data)
{
	_bodyData.append(data);
}

void HttpRequest::decodeBody()
{
	if (body().isEmpty())
		return;
	
	QByteArray contentType = header("content-type").toLower();
	static QRegularExpression codecRegex(".+;\\s*encoding=(\\S+).*");
	QRegularExpressionMatch codecMatch = codecRegex.match(contentType);
	QTextCodec *codec = QTextCodec::codecForName(codecMatch.hasMatch() ? codecMatch.captured(1).toLatin1() : "iso-8859-15");
	QString content = codec->toUnicode(body());
	
	if (contentType.startsWith("application/x-www-form-urlencoded"))
	{
		QStringList params = content.split('&');
		for (QString param : params)
		{
			QString name, value;
			int equals = param.indexOf('=');
			if (equals >= 0)
			{
				name = param.mid(0, equals);
				value = param.mid(equals+1);
			}
			else
				name = param;
			_parameters.insert(decode(name.toUtf8()), decode(value.toUtf8()));
		}
	}
	
	else if (contentType.startsWith("application/json"))
	{
		QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8());
		if (doc.isObject())
		{
			QJsonObject obj = doc.object();
			for (QString key : obj.keys())
			{
				QJsonValue val = obj.value(key);
				if (!val.isArray() && !val.isObject())
					_parameters.insert(key.toUtf8(), val.toVariant().toByteArray());
				else
					qWarning() << "received json body that contains nested elements";
			}
		}
		else
			qWarning() << "received json body which is not an json object";
	}
	
	else if (contentType.startsWith("multipart/form-data"))
	{
		static QRegularExpression regex("^multipart\\/form-data(;.*)*;\\s*boundary=(?P<boundary>\\S+).*", QRegularExpression::CaseInsensitiveOption);
		QRegularExpressionMatch match = regex.match(contentType);
		if (!match.hasMatch())
		{
			qWarning() << "received multipart without boundary: " << contentType;
			return;
		}
		QString boundary = match.captured("boundary").toLower();
		static QRegularExpression dispRegex("^Content-Disposition:\\s+(?P<type>\\S+)(;.*)*;\\s*name=\"(?P<name>[^\"]+)\".*", QRegularExpression::CaseInsensitiveOption);
		QByteArray fieldName, fileName;
		QByteArray val;
		QTemporaryFile *file = 0;
		QString line;
		int index = -1;
		int lastIndex = 0;
		while ((index = content.indexOf("\r\n", lastIndex)) >= 0)
		{
			line = content.mid(lastIndex, index - lastIndex);
			lastIndex = index + 2;
			
			if (line.toLower().startsWith("--" + boundary))
			{
				if (!fieldName.isEmpty())
					_parameters.insert(fieldName, val);
				fieldName = "";
				fileName = "";
				val = "";
				if (file)
				{
					file->close();
					delete file;
				}
				file = 0;
			}
			else if (line.toLower().startsWith("content-disposition"))
			{
				match = dispRegex.match(line);
				if (!match.hasMatch())
					qWarning() << "Unknown Content-Disposition Header: " << line;
				else if (match.captured("type") == "form-data")
					fieldName = codec->fromUnicode(match.captured("name"));
				else
					qWarning() << "Unknown type in Content-Disposition Header: " << line;
			}
			else if (fileName.isEmpty())
				val += codec->fromUnicode(line);
			else
			{
				if (!file)
				{
					file = new QTemporaryFile;
					_uploadedFiles.insert(fieldName, file);
					file->open();
				}
				file->write(codec->fromUnicode(line) + "\r\n");
			}
		}
		
		if (!fieldName.isEmpty())
			_parameters.insert(fieldName, val);
		if (file)
		{
			file->close();
			delete file;
		}
		
		qDebug() << "params:" << _parameters;
		qDebug() << "files :" << _uploadedFiles;
	}
	
	else
	{
		qWarning() << "received body with unknown content type " << contentType;
#ifdef QT_DEBUG
		qWarning() << content;
#endif
	}
}

QByteArray HttpRequest::decode(const QByteArray &in) const
{
	QByteArray out(in);
	out.replace('+', ' ');
	int index;
	do
	{
		index = out.indexOf('%');
		out.replace(index, 3, QByteArray().append((char)(out.mid(index+1,2).toInt(0,16))));
	}
	while (index >= 0);
	return out;
}

void HttpRequest::setPath(const QByteArray &path)
{
	_path = decode(path);
}

QTemporaryFile *HttpRequest::uploadedFile(const QByteArray fieldName)
{
	return _uploadedFiles.value(fieldName);
}

QByteArray HttpRequest::cookie(const QByteArray &name) const
{
	return _cookies.value(name);
}

/** Get the map of cookies */
QMap<QByteArray, QByteArray> &HttpRequest::cookieMap()
{
	return _cookies;
}

