/**
  @file
  @author Stefan Frings
*/

#include "httprequesthandler.h"

HttpRequestHandler::HttpRequestHandler(QObject *parent)
	: QObject(parent)
{}

HttpRequestHandler::~HttpRequestHandler() {}

void HttpRequestHandler::service(HttpRequest &request, HttpResponse &response)
{
	qCritical("HttpRequestHandler: you need to override the service() function");
	qDebug() << "HttpRequestHandler: request=" << request.method() << request.path() << request.protocol();
	response.setStatus(NOT_IMPLEMENTED);
	response.write("501 not implemented", true);
}
