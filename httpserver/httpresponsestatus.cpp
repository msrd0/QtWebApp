/*
 * 
 * 
 * Copyright (C) 2015 Dominic S. Meiser <meiserdo@web.de>
 * 
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or any later
 * version.
 * 
 * This work is distributed in the hope that it will be useful, but without
 * any warranty; without even the implied warranty of merchantability or
 * fitness for a particular purpose. See version 2 and version 3 of the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "httpresponsestatus.h"

HttpResponseStatus::HttpResponseStatus(int code, const QByteArray &text, HttpRequest::Protocol since)
	: QObject(0)
	, _code(code)
	, _text(text)
	, _since(since)
	, _fallback(0)
{
}

HttpResponseStatus::HttpResponseStatus(int code, const QByteArray &text, HttpRequest::Protocol since, const HttpResponseStatus &fallback)
	: QObject(0)
	, _code(code)
	, _text(text)
	, _since(since)
	, _fallback(new HttpResponseStatus(fallback))
{
}

HttpResponseStatus::HttpResponseStatus(const HttpResponseStatus &other)
	: QObject(0)
	, _code(other._code)
	, _text(other._text)
	, _since(other._since)
	, _fallback(other._fallback)
{
}

void HttpResponseStatus::operator= (const HttpResponseStatus &other)
{
	_code = other._code;
	_text = other._text;
	_since = other._since;
	_fallback = other._fallback;
}

HttpResponseStatus HttpResponseStatus::status (HttpRequest::Protocol protocol) const
{
	if (protocol >= _since)
		return *this;
	if (_fallback.data())
		return _fallback.data()->status(protocol);
	else return HTTP_VERSION_NOT_SUPPORTED;
}
