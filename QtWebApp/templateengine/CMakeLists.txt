set(templateengine_HEADERS
		templatecache.h
		templateglobal.h
		template.h
		templateloader.h
	)
set(templateengine_SOURCES
		templatecache.cpp
		template.cpp
		templateloader.cpp
	)

add_library(QtWebAppTemplateEngine SHARED ${templateengine_HEADERS} ${templateengine_SOURCES})
target_link_libraries(QtWebAppTemplateEngine Qt5::Core Qt5::Network)
set_target_properties(QtWebAppTemplateEngine PROPERTIES
		VERSION ${qtwebapp_VERSION}
		SOVERSION ${qtwebapp_MAJOR}
	)

install(TARGETS QtWebAppTemplateEngine
#         EXPORT QtWebAppTemplateEngine
        LIBRARY DESTINATION lib)
install(FILES ${templateengine_HEADERS}
        DESTINATION include/qtwebapp/templateengine)
# install(EXPORT QtWebAppTemplateEngine
#         DESTINATION /usr/lib/cmake/QtWebAppTemplateEngine)
