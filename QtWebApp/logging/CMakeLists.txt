set(logging_HEADERS
		dualfilelogger.h
		filelogger.h
		logger.h
		logmessage.h
	)
set(logging_SOURCES
		dualfilelogger.cpp
		filelogger.cpp
		logger.cpp
		logmessage.cpp
	)

add_library(QtWebAppLogging SHARED ${logging_HEADERS} ${logging_SOURCES})
target_include_directories(QtWebAppLogging PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include/qtwebapp/logging>
)
target_link_libraries(QtWebAppLogging Qt5::Core Qt5::Network)
set_target_properties(QtWebAppLogging PROPERTIES
		VERSION ${qtwebapp_VERSION}
		SOVERSION ${qtwebapp_MAJOR}
	)

install(TARGETS QtWebAppLogging
        EXPORT QtWebAppTargets
        LIBRARY DESTINATION lib)
install(FILES ${logging_HEADERS}
        DESTINATION include/qtwebapp/logging)
