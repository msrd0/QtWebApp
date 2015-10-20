# Build this project to generate a shared library (*.dll or *.so).

TARGET = QtWebApp
TEMPLATE = lib
QT -= gui
QT += network
VERSION = 1.5.8

mac {
   QMAKE_MAC_SDK = macosx10.10
   QMAKE_CXXFLAGS += -std=c++11
   QMAKE_LFLAGS_SONAME  = -Wl,-install_name,/usr/local/lib/
}

win32 {
   DEFINES += QTWEBAPPLIB_EXPORT
}

DISTFILES += doc/* mainpage.dox Doxyfile

include(qtservice/qtservice.pri)
include(logging/logging.pri)
include(httpserver/httpserver.pri)
include(templateengine/templateengine.pri)
