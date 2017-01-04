set(httpserver_HEADERS
		httpconnectionhandler.h
		httpconnectionhandlerpool.h
		httpcookie.h
		httplistener.h
		httpserverconfig.h
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
		httplistener.cpp
		httpserverconfig.cpp
		httprequest.cpp
		httpresponse.cpp
		httpsession.cpp
		httpsessionstore.cpp
		staticfilecontroller.cpp
	)

add_library(QtWebAppHttpServer SHARED ${httpserver_HEADERS} ${httpserver_SOURCES})
target_link_libraries(QtWebAppHttpServer QtWebAppGlobal Qt5::Core Qt5::Network)
set_target_properties(QtWebAppHttpServer PROPERTIES
		VERSION ${qtwebapp_VERSION}
		SOVERSION ${qtwebapp_MAJOR}
	)

install(TARGETS QtWebAppHttpServer
#         EXPORT QtWebAppHttpServer
		RUNTIME DESTINATION bin
		ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(FILES ${httpserver_HEADERS}
        DESTINATION include/qtwebapp/httpserver)
# install(EXPORT QtWebAppHttpServer
#         DESTINATION /usr/lib/cmake/QtWebAppHttpServer)
