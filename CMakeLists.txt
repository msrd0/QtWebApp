cmake_minimum_required(VERSION 3.1)
project(QtWebApp CXX)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED HINTS $ENV{Qt6_DIR} $ENV{Qt5_DIR})

add_subdirectory(QtWebApp)
