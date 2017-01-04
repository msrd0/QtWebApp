set(templateengine_HEADERS
		template.h
		templatecache.h
		templateengineconfig.h
		templateloader.h
	)
set(templateengine_SOURCES
		template.cpp
		templatecache.cpp
		templateengineconfig.cpp
		templateloader.cpp
	)

add_library(QtWebAppTemplateEngine SHARED ${templateengine_HEADERS} ${templateengine_SOURCES})
target_link_libraries(QtWebAppTemplateEngine QtWebAppGlobal Qt5::Core Qt5::Network)
set_target_properties(QtWebAppTemplateEngine PROPERTIES
		VERSION ${qtwebapp_VERSION}
		SOVERSION ${qtwebapp_MAJOR}
	)

install(TARGETS QtWebAppTemplateEngine
#         EXPORT QtWebAppTemplateEngine
		RUNTIME DESTINATION bin
		ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)
install(FILES ${templateengine_HEADERS}
        DESTINATION include/qtwebapp/templateengine)
# install(EXPORT QtWebAppTemplateEngine
#         DESTINATION /usr/lib/cmake/QtWebAppTemplateEngine)
