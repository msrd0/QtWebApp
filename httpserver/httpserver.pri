INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += network

HEADERS += $$PWD/httpglobal.h \
           $$PWD/httplistener.h \
           $$PWD/httpconnectionhandler.h \
           $$PWD/httpconnectionhandlerpool.h \
           $$PWD/httprequest.h \
           $$PWD/httpresponse.h \
           $$PWD/httpcookie.h \
           $$PWD/httprequesthandler.h \
           $$PWD/httpsession.h \
           $$PWD/httpsessionstore.h \
           $$PWD/staticfilecontroller.h \
           $$PWD/httpstream.h \
    $$PWD/hpack.h

SOURCES += $$PWD/httpglobal.cpp \
           $$PWD/httplistener.cpp \
           $$PWD/httpconnectionhandler.cpp \
           $$PWD/httpconnectionhandlerpool.cpp \
           $$PWD/httprequest.cpp \
           $$PWD/httpresponse.cpp \
           $$PWD/httpcookie.cpp \
           $$PWD/httprequesthandler.cpp \
           $$PWD/httpsession.cpp \
           $$PWD/httpsessionstore.cpp \
           $$PWD/staticfilecontroller.cpp \
           $$PWD/httpstream.cpp \
    $$PWD/httpstream-http1.cpp \
    $$PWD/httpstream-http2.cpp \
    $$PWD/hpack.cpp

DISTFILES += \
    $$PWD/hpack_static_table.txt

RESOURCES += \
    $$PWD/hpack.qrc
