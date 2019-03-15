cmake_minimum_required(VERSION 3.12)
project(fscanner)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)

#add_executable(fscaner main.cpp mainwindow.h mainwindow.cpp scanner.h scanner.cpp my_file.h my_file.cpp)
add_executable(fscanner main.cpp mainwindow.h mainwindow.cpp scanner.h scanner.cpp scanfile.cpp scanfile.h)
qt5_use_modules(fscanner Widgets)
