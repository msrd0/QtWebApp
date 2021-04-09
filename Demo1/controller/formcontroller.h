/**
  @file
  @author Stefan Frings
*/

#ifndef FORMCONTROLLER_H
#define FORMCONTROLLER_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

/**
  This controller displays a HTML form and dumps the submitted input.
*/

class FormController : public qtwebapp::HttpRequestHandler {
	Q_OBJECT
	Q_DISABLE_COPY(FormController)
  public:
	/** Constructor */
	FormController();

	/** Generates the response */
	void service(qtwebapp::HttpRequest &request, qtwebapp::HttpResponse &response);
};

#endif // FORMCONTROLLER_H
