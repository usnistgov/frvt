set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
include_directories (${CMAKE_SOURCE_DIR}/src/include)

# Configure to put executable in top level bin directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Get library implementation name
set (FRVT_IMPL_LIB $ENV{FRVT_IMPL_LIB})

# Build executable link to dependent libraries
add_executable (validate11 util.cpp validate11.cpp)
target_link_libraries (validate11 ${FRVT_IMPL_LIB})
