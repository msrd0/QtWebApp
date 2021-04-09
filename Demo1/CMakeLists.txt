cmake_minimum_required(VERSION 2.8.11)
project("QtWebApp Demo 1")

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Network)
find_package(QtWebApp REQUIRED COMPONENTS HttpServer Logging TemplateEngine)

add_executable(demo1
	controller/dumpcontroller.cpp
	controller/dumpcontroller.h
	controller/fileuploadcontroller.cpp
	controller/fileuploadcontroller.h
	controller/formcontroller.cpp
	controller/formcontroller.h
	controller/logincontroller.cpp
	controller/logincontroller.h
	controller/sessioncontroller.cpp
	controller/sessioncontroller.h
	controller/templatecontroller.cpp
	controller/templatecontroller.h
	documentcache.h
	global.cpp
	global.h
	main.cpp
	requestmapper.cpp
	requestmapper.h
)
target_link_libraries(demo1 ${QtWebApp_LIBRARIES})
