/**
  @file
  @author Stefan Frings
*/

#ifndef LOGINCONTROLLER_H
#define LOGINCONTROLLER_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

/**
  This controller demonstrates how to use HTTP basic login.
*/

class LoginController : public qtwebapp::HttpRequestHandler {
	Q_OBJECT
	Q_DISABLE_COPY(LoginController)
  public:
	/** Constructor */
	LoginController();

	/** Generates the response */
	void service(qtwebapp::HttpRequest &request, qtwebapp::HttpResponse &response);
};

#endif // LOGINCONTROLLER_H
