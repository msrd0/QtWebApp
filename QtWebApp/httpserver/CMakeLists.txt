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
target_include_directories(QtWebAppHttpServer PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include/qtwebapp/httpserver>
)
target_link_libraries(QtWebAppHttpServer QtWebAppGlobal Qt5::Core Qt5::Network)
set_target_properties(QtWebAppHttpServer PROPERTIES
		VERSION ${qtwebapp_VERSION}
		SOVERSION ${qtwebapp_MAJOR}
	)

install(TARGETS QtWebAppHttpServer
        EXPORT QtWebAppTargets
        LIBRARY DESTINATION lib)
install(FILES ${httpserver_HEADERS}
        DESTINATION include/qtwebapp/httpserver)
