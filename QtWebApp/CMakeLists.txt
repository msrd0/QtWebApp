cmake_minimum_required(VERSION 3.1)
project(QtWebApp)

set(qtwebapp_MAJOR 1)
set(qtwebapp_MINOR 7)
set(qtwebapp_PATCH 1)
set(qtwebapp_VERSION ${qtwebapp_MAJOR}.${qtwebapp_MINOR}.${qtwebapp_PATCH})

find_package(Qt5 REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)

add_definitions(-DQTWEBAPPLIB_EXPORT)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Werror=format -Werror=return-type")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=format -Werror=return-type")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=undefined")
	add_definitions("-D_GLIBCXX_DEBUG")
	add_definitions("-DQT_SHAREDPOINTER_TRACK_POINTERS")
	add_definitions("-DCMAKE_DEBUG")
	add_definitions("-DSUPERVERBOSE")
endif()
add_definitions("-DCMAKE_QTWEBAPP_SO")

configure_file(qtwebappglobal.h.in qtwebappglobal.h @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtwebappglobal.h
	    DESTINATION include/qtwebapp/)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(QtWebAppGlobal SHARED qtwebappglobal.cpp)
target_link_libraries(QtWebAppGlobal Qt5::Core)
set_target_properties(QtWebAppGlobal PROPERTIES
		VERSION ${qtwebapp_VERSION}
		SOVERSION ${qtwebapp_MAJOR}
	)
install(TARGETS QtWebAppGlobal
		RUNTIME DESTINATION bin
		ARCHIVE DESTINATION lib
	    LIBRARY DESTINATION lib)

add_subdirectory(logging)
add_subdirectory(httpserver)
add_subdirectory(templateengine)

configure_file(cmake/QtWebAppConfig.cmake.in        QtWebAppConfig.cmake        @ONLY)
configure_file(cmake/QtWebAppConfigVersion.cmake.in QtWebAppConfigVersion.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QtWebAppConfig.cmake
	          ${CMAKE_CURRENT_BINARY_DIR}/QtWebAppConfigVersion.cmake
        DESTINATION lib/cmake/QtWebApp)
