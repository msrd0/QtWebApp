#pragma once

#include <QVariant>
#include <QtGlobal>
#include <sys/types.h>

#define QTWEBAPP_MAJOR @qtwebapp_MAJOR@
#define QTWEBAPP_MINOR @qtwebapp_MINOR@
#define QTWEBAPP_PATCH @qtwebapp_PATCH@
#define QTWEBAPP_VERSION ((@qtwebapp_MAJOR@ << 16) | (@qtwebapp_MINOR@ << 8) | @qtwebapp_PATCH@)
#define QTWEBAPP_VERSION_STR "@qtwebapp_VERSION@"

#ifdef CMAKE_QTWEBAPP_SO
#define QTWEBAPP_EXPORT Q_DECL_EXPORT
#else
#define QTWEBAPP_EXPORT Q_DECL_IMPORT
#endif

namespace qtwebapp {

	/// The version of QtWebApp.
	QTWEBAPP_EXPORT const char *getQtWebAppLibVersion();

	/// Parses the given number by respecting its suffix.
	QTWEBAPP_EXPORT int parseNum(const QVariant &v, int base = 1e3);

} // namespace qtwebapp
