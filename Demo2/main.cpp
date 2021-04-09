/**
  @file
  @author Stefan Frings
*/

#include "requesthandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <httpserver/httplistener.h>
#include <httpserver/httpserverconfig.h>
#include <logging/filelogger.h>

using namespace qtwebapp;

/**
  Entry point of the program.
*/
int main(int argc, char *argv[]) {

	// Initialize the core application
	QCoreApplication app(argc, argv);
	app.setApplicationName("Demo2");

	// Configure and start the TCP listener
	HttpServerConfig conf;
	conf.port = 8080;
	conf.minThreads = 4;
	conf.maxThreads = 100;
	conf.cleanupInterval = 60e3;
	conf.readTimeout = 60e3;
	conf.maxRequestSize = 16e3;
	conf.maxMultipartSize = 10e6;
	new HttpListener(conf, new RequestHandler(&app), &app);

	qWarning("Application has started");
	app.exec();
	qWarning("Application has stopped");
}
