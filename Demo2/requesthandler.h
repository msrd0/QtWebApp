/**
  @file
  @author Stefan Frings
*/

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <httpserver/httprequesthandler.h>

/**
  The request handler receives incoming HTTP requests and generates responses.
*/

class RequestHandler : public qtwebapp::HttpRequestHandler {
	Q_OBJECT
	Q_DISABLE_COPY(RequestHandler)
  public:
	/**
	  Constructor.
	  @param parent Parent object
	*/
	RequestHandler(QObject *parent = 0);

	/**
	  Destructor
	*/
	~RequestHandler();

	/**
	  Process an incoming HTTP request.
	  @param request The received HTTP request
	  @param response Must be used to return the response
	*/
	void service(qtwebapp::HttpRequest &request, qtwebapp::HttpResponse &response);
};

#endif // REQUESTHANDLER_H
