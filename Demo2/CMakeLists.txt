cmake_minimum_required(VERSION 2.8.11)
project("QtWebApp Demo 2")

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Network)
find_package(QtWebApp REQUIRED COMPONENTS HttpServer Logging)

add_executable(demo2
	main.cpp
	requesthandler.cpp
	requesthandler.h
)
target_link_libraries(demo2 ${QtWebApp_LIBRARIES})
