/**
  @file
  @author Stefan Frings
*/

#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

/**
  This controller demonstrates how to use sessions.
*/

class SessionController : public qtwebapp::HttpRequestHandler {
	Q_OBJECT
	Q_DISABLE_COPY(SessionController)
  public:
	/** Constructor */
	SessionController();

	/** Generates the response */
	void service(qtwebapp::HttpRequest &request, qtwebapp::HttpResponse &response);
};

#endif // SESSIONCONTROLLER_H
