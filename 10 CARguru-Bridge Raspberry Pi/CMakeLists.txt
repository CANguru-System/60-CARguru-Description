cmake_minimum_required(VERSION 3.0.0)
project(CARguru VERSION 0.1.0)

#Bring the headers, such as Student.h into the project
include_directories(include)

#Can manually add the sources using the set command as follows:
#set(SOURCES src/main.cpp)

#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.c*")

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})
link_libraries(${GTKMM_LIBRARIES})

# Create a variable that holds the path to our libwiringPi.so file
#set (WPI_PATH_LIB /usr/lib/wiringPi)
#set (WPI_PATH_INCLUDE /usr/include/wiringPi)
# Add the local ‘include’ directory and the wiringPi directory to grab headers
#include_directories (include ${WPI_PATH_INCLUDE})
# Actually find the wiringPi library object
#find_library(WPI_LIB wiringPi HINTS ${WPI_PATH_LIB} NO_CMAKE_FIND_ROOT_PATH)
# Link the pre-compiled wiringPi library to the executable we just declared 
#link_libraries(${WPI_LIB})

find_library(wiringPi_LIB wiringPi)
link_libraries(${wiringPi_LIB})

include(CTest)
enable_testing()

add_executable(CARguru ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
