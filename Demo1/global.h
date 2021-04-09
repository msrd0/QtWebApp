/**
  @file
  @author Stefan Frings
*/

#ifndef GLOBAL_H
#define GLOBAL_H

#include "filelogger.h"
#include "httpsessionstore.h"
#include "staticfilecontroller.h"
#include "templatecache.h"

/**
  Global objects that are shared by multiple source files
  of this project.
*/

/** Cache for template files */
extern qtwebapp::TemplateCache *templateCache;

/** Storage for session cookies */
extern qtwebapp::HttpSessionStore *sessionStore;

/** Controller for static files */
extern qtwebapp::StaticFileController *staticFileController;

/** Redirects log messages to a file */
extern qtwebapp::FileLogger *logger;

#endif // GLOBAL_H
