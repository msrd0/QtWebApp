cmake_minimum_required(VERSION 3.22)
project(QtWebApp CXX)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED HINTS $ENV{Qt6_DIR} $ENV{Qt5_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT MSVC)
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Werror=format -Werror=return-type")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=format -Werror=return-type")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	if(MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=undefined")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
	endif()
	add_definitions("-D_GLIBCXX_DEBUG")
	add_definitions("-DQT_SHAREDPOINTER_TRACK_POINTERS")
	add_definitions("-DCMAKE_DEBUG")
	add_definitions("-DSUPERVERBOSE")
endif()

set(CMAKE_AUTOMOC ON)

add_subdirectory(QtWebApp)
