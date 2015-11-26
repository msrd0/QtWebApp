/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPGLOBAL_H
#define HTTPGLOBAL_H

#include <QtGlobal>

// This is specific to Windows dll's
#ifdef Q_OS_WIN
#  if defined QTWEBAPPLIB_EXPORT
#    define DECLSPEC Q_DECL_EXPORT
#  elif defined QTWEBAPPLIB_IMPORT
#    define DECLSPEC Q_DECL_IMPORT
#  endif
#endif
#ifndef DECLSPEC
#  define DECLSPEC
#endif

/** Get the library version number */
DECLSPEC const char *getQtWebAppLibVersion();

// fix for qt versions below 5.5
#ifndef Q_ENUM
#  define Q_ENUM(x) Q_ENUMS(x)
#endif

#endif // HTTPGLOBAL_H

