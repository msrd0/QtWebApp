cmake_minimum_required(VERSION 2.8.11)
project(QtWebApp)

set(qtwebapp_MAJOR 1)
set(qtwebapp_MINOR 7)
set(qtwebapp_PATCH 1)
set(qtwebapp_VERSION ${qtwebapp_MAJOR}.${qtwebapp_MINOR}.${qtwebapp_PATCH})

find_package(Qt5 REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)

add_definitions(-DQTWEBAPPLIB_EXPORT)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -DSUPERVERBOSE")
# just to avoid any further issues with this - it might work with older versions, but
# it's hard to find a compiler that doesn't support it nowadays
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(logging)
add_subdirectory(httpserver)
add_subdirectory(templateengine)

configure_file(cmake/QtWebAppConfig.cmake.in        QtWebAppConfig.cmake        @ONLY)
configure_file(cmake/QtWebAppConfigVersion.cmake.in QtWebAppConfigVersion.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QtWebAppConfig.cmake
	          ${CMAKE_CURRENT_BINARY_DIR}/QtWebAppConfigVersion.cmake
        DESTINATION lib/cmake/QtWebApp)
