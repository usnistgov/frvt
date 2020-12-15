set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -DNIST_EXTERN_FRVT_STRUCTS_VERSION -DNIST_EXTERN_API_VERSION")
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/src/include)

# Configure to put executable in top level bin directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Get library implementation name
set (FRVT_IMPL_LIB $ENV{FRVT_IMPL_LIB})

# Build executable link to dependent libraries
add_executable (validate_morph ../../../common/src/util/util.cpp validate_morph.cpp)
target_link_libraries (validate_morph ${FRVT_IMPL_LIB})
