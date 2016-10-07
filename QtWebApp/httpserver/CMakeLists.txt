set(httpserver_HEADERS
		httpconnectionhandler.h
		httpconnectionhandlerpool.h
		httpcookie.h
		httpglobal.h
		httplistener.h
		httprequest.h
		httprequesthandler.h
		httpresponse.h
		httpsession.h
		httpsessionstore.h
		staticfilecontroller.h
	)
set(httpserver_SOURCES
		httpconnectionhandler.cpp
		httpconnectionhandlerpool.cpp
		httpcookie.cpp
		httpglobal.cpp
		httplistener.cpp
		httprequest.cpp
		httprequesthandler.cpp
		httpresponse.cpp
		httpsession.cpp
		httpsessionstore.cpp
		staticfilecontroller.cpp
	)

add_library(QtWebAppHttpServer SHARED ${httpserver_HEADERS} ${httpserver_SOURCES})
target_link_libraries(QtWebAppHttpServer Qt5::Core Qt5::Network)
set_target_properties(QtWebAppHttpServer PROPERTIES
		VERSION ${qtwebapp_VERSION}
		SOVERSION ${qtwebapp_MAJOR}
	)

install(TARGETS QtWebAppHttpServer
#         EXPORT QtWebAppHttpServer
        LIBRARY DESTINATION lib)
install(FILES ${httpserver_HEADERS}
        DESTINATION include/qtwebapp/httpserver)
# install(EXPORT QtWebAppHttpServer
#         DESTINATION /usr/lib/cmake/QtWebAppHttpServer)
