# Build this project to generate a shared library (*.dll or *.so).

TARGET = QtWebApp
TEMPLATE = lib
QT -= gui
QT += network
CONFIG += c++11

VERSION = 1.5.8

clang {
	QMAKE_CXXFLAGS_RELEASE -= -fvar-tracking-assignments -Og
}

mac {
   QMAKE_MAC_SDK = macosx10.10
   QMAKE_LFLAGS_SONAME  = -Wl,-install_name,/usr/local/lib/
}

win32 {
   DEFINES += QTWEBAPPLIB_EXPORT
}

DISTFILES += doc/* mainpage.dox Doxyfile

include(logging/logging.pri)
include(httpserver/httpserver.pri)
include(templateengine/templateengine.pri)
