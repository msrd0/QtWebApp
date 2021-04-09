/**
  @file
  @author Stefan Frings
*/

#ifndef DUMPCONTROLLER_H
#define DUMPCONTROLLER_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

/**
  This controller dumps the received HTTP request in the response.
*/

class DumpController : public qtwebapp::HttpRequestHandler {
	Q_OBJECT
	Q_DISABLE_COPY(DumpController)
  public:
	/** Constructor */
	DumpController();

	/** Generates the response */
	void service(qtwebapp::HttpRequest &request, qtwebapp::HttpResponse &response);
};

#endif // DUMPCONTROLLER_H
