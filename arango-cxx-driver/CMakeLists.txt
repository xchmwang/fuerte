################################################################################
### DISCLAIMER
###
### Copyright 2016 ArangoDB GmbH, Cologne, Germany
###
### Licensed under the Apache License, Version 2.0 (the "License");
### you may not use this file except in compliance with the License.
### You may obtain a copy of the License at
###
###     http://www.apache.org/licenses/LICENSE-2.0
###
### Unless required by applicable law or agreed to in writing, software
### distributed under the License is distributed on an "AS IS" BASIS,
### WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
### See the License for the specific language governing permissions and
### limitations under the License.
###
### Copyright holder is ArangoDB GmbH, Cologne, Germany
###
### @author Frank Celler
### @author Jan Uhde
### @author John Bufton
################################################################################

cmake_minimum_required(VERSION 3.6)

project(fuerte CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
  # -fPIC here as curlpp is broken
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

################################################################################
## Configuration
################################################################################

option(FUERTE_TESTS    "Build Tests" OFF)
option(FUERTE_EXAMPLES "Build EXAMPLES" OFF)

################################################################################
## Dependencies
################################################################################

find_package(Threads REQUIRED)

set(CMAKE_BOOST_COMPONENTS "system")
find_package(Boost COMPONENTS ${CMAKE_BOOST_COMPONENTS})

if(Boost_FOUND)
    message(STATUS "== Using Boost provided by your system")
else()
    add_subdirectory(3rdParty/cmake-for-boost)
endif()

option(BuildVelocyPackExamples "Build examples" OFF)
add_subdirectory(3rdParty/velocypack)
add_subdirectory(3rdParty/curlpp2)

################################################################################
## Main Project
################################################################################

add_library(fuerte-old STATIC EXCLUDE_FROM_ALL
    src/old/ConnectionUrl.cpp
    src/old/HttpConnection.cpp
    src/old/VppConnection.cpp
    src/old/Connection.cpp
    src/old/ConOption.cpp
    src/old/Server.cpp
    src/old/Database.cpp
    src/old/Collection.cpp
    src/old/Document.cpp
    src/old/Cursor.cpp
    src/old/Version.cpp
    src/old/HeaderCommon.cpp
    src/old/HeaderMulti.cpp
    src/old/HeaderLoader.cpp
)

target_include_directories(fuerte-old PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/curlpp2/include
)

target_link_libraries(fuerte-old PUBLIC
    velocypack
    curlpp
    Boost::system
)
target_compile_definitions(fuerte-old PUBLIC
    BOOST_ASIO_DISABLE_THREADS
)

# FIXME -FPIC does not end up in curlpp
set_target_properties(fuerte-old velocypack curlpp PROPERTIES POSITION_INDEPENDENT_CODE ON)
#DVELOCYPACK_XXHASH=1

## fuerte-next
add_library(fuerte STATIC
    src/ConnectionBuilder.cpp
    src/HttpCommunicator.cpp
    src/HttpConnection.cpp
    src/VstConnection.cpp
    src/LoopProvider.cpp
    src/Request.cpp
    src/asio.cpp
    src/Connection.cpp
)

target_link_libraries(fuerte PUBLIC
    velocypack
    curlpp
    Boost::system
)

target_include_directories(fuerte PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/curlpp2/include
)

add_executable(fuerte-get
  tools/fuerte-get.cpp
)

target_link_libraries(fuerte-get
  fuerte
  ${CMAKE_THREAD_LIBS_INIT}
)

#########################################################################################
# Tests
if(FUERTE_TESTS)
#if(CMAKE_TESTING)
    add_subdirectory(3rdParty/googletest)
    add_subdirectory(tests)
endif()

#########################################################################################
# Examples
if(FUERTE_EXAMPLES)
    if(not TARGET gtest)
        add_subdirectory(3rdParty/googletest) # should not be required by examples
    endif()
    add_subdirectory(examples)
endif()

#########################################################################################
# Clang Completion
#copy compile commands to source dir
if( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
    message(STATUS "copy compile_commands.json")
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()


#########################################################################################
# Install
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(UNIX)
        set(CMAKE_INSTALL_PREFIX  "$ENV{HOME}/local")
    else()
        message("not implemented for other operating systems")
    endif()
endif()

install(TARGETS fuerte DESTINATION lib)

install(
    DIRECTORY   "include/fuerte"
    DESTINATION include
)

include(CPack) #should be last command
