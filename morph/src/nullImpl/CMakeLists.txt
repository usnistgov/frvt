cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Release)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/src/include)

# Configure built shared libraries in top-level lib directory
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

# Build the shared libraries
add_library (frvt_morph_null_001 SHARED nullimplfrvtmorph.cpp)
