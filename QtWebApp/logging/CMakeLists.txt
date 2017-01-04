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
target_link_libraries(QtWebAppLogging Qt5::Core Qt5::Network)
set_target_properties(QtWebAppLogging PROPERTIES
		VERSION ${qtwebapp_VERSION}
		SOVERSION ${qtwebapp_MAJOR}
	)

install(TARGETS QtWebAppLogging
#         EXPORT QtWebAppLogging
		RUNTIME DESTINATION bin
		ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(FILES ${logging_HEADERS}
        DESTINATION include/qtwebapp/logging)
# install(EXPORT QtWebAppLogging
#         DESTINATION /usr/lib/cmake/QtWebAppLogging)
