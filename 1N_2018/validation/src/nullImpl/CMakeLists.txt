set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories (${CMAKE_SOURCE_DIR}/src/include ${CMAKE_CURRENT_SOURCE_DIR})

# Configure built shared libraries in top-level lib directory
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Build shared 1:N library
add_library (frvt1N_null_0 SHARED nullimplfrvt1N.cpp)
