/**
  @file
  @author Stefan Frings
*/

#ifndef FILEUPLOADCONTROLLER_H
#define FILEUPLOADCONTROLLER_H

#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"

/**
  This controller displays a HTML form for file upload and recieved the file.
*/

class FileUploadController : public qtwebapp::HttpRequestHandler {
	Q_OBJECT
	Q_DISABLE_COPY(FileUploadController)
  public:
	/** Constructor */
	FileUploadController();

	/** Generates the response */
	void service(qtwebapp::HttpRequest &request, qtwebapp::HttpResponse &response);
};

#endif // FILEUPLOADCONTROLLER_H
