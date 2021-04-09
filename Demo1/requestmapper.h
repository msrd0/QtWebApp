/**
  @file
  @author Stefan Frings
*/

#ifndef REQUESTMAPPER_H
#define REQUESTMAPPER_H

#include "httprequesthandler.h"

/**
  The request mapper dispatches incoming HTTP requests to controller classes
  depending on the requested path.
*/

class RequestMapper : public qtwebapp::HttpRequestHandler {
	Q_OBJECT
	Q_DISABLE_COPY(RequestMapper)
  public:
	/**
	  Constructor.
	  @param parent Parent object
	*/
	RequestMapper(QObject *parent = 0);

	/**
	  Destructor.
	*/
	~RequestMapper();

	/**
	  Dispatch incoming HTTP requests to different controllers depending on the URL.
	  @param request The received HTTP request
	  @param response Must be used to return the response
	*/
	void service(qtwebapp::HttpRequest &request, qtwebapp::HttpResponse &response);
};

#endif // REQUESTMAPPER_H
