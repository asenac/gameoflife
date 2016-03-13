cmake_minimum_required(VERSION 3.4)
project(conway CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "-Wall")

find_package(Qt4 REQUIRED QtGui QtCore)
include_directories(${QT_INCLUDES})

add_executable(conway MACOSX_BUNDLE main.cpp widgets.hpp)
target_link_libraries(conway Qt4::QtGui)

