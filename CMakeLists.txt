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
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/boost-include/")
include_directories("${PROJECT_SOURCE_DIR}/beast-include/")
include_directories("${PROJECT_SOURCE_DIR}/json-single-include/")

# set the sources
file(GLOB ROOT_HEADERS "*.h")
file(GLOB ROOT_SOURCES "*.cpp")

#source_group("Header Files\\${ROOT_HEADERS})
#source_group("Source Files\\${ROOT_SOURCES})
link_directories("/usr/lib/x86_64-linux-gnu/")

set(SOURCES ${ROOT_HEADERS} ${ROOT_SOURCES})
add_library(restlib SHARED ${SOURCES})

target_link_libraries(restlib -lboost_thread -lboost_system -lcrypto -lz -lpthread -lssl)

install(TARGETS restlib LIBRARY DESTINATION /usr/local/lib)
