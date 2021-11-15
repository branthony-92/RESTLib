cmake_minimum_required(VERSION 3.9)
project(RESTLib)

# Version info
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (VERSION_BUILD 0)
set (VERSION_REV   0)
set (VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}-${VERSION_REV})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")
# add_compile_options(/W4 /bigobj)

# include the current directory
include_directories("/usr/include/")
include_directories("/usr/include/json/include")
include_directories("/usr/include/x86_64-linux-gnu/")
include_directories("${PROJECT_SOURCE_DIR}")

# set the sources
file(GLOB ROOT_HEADERS "*.h")
file(GLOB ROOT_SOURCES "*.cpp")

link_directories("/lib/")
link_directories("/lib/x86_64-linux-gnu/")
link_directories("/usr/lib/")
link_directories("/usr/lib/x86_64-linux-gnu/")
link_directories("/usr/local/lib/")

set(SOURCES ${ROOT_HEADERS} ${ROOT_SOURCES})
add_library(restlib ${SOURCES})

target_link_libraries(restlib -lboost_thread-static -lboost_system-static -lcrypto -lz -lpthread -lssl)

install(TARGETS LIBRARY DESTINATION "/usr/local/lib/")
install(FILES ${ROOT_HEADERS} ${ROOT_SOURCES} DESTINATION /usr/local/include/restlib/include)