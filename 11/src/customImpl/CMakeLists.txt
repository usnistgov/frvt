cmake_minimum_required(VERSION 2.8)

project(frvt_11_custom_001)

ADD_DEFINITIONS(-DLINUX)

find_package( OpenCV REQUIRED )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings")

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}/../include 
	${CMAKE_CURRENT_SOURCE_DIR}/../../../common/src/include
	${CMAKE_CURRENT_SOURCE_DIR}/../../external/msnet-mtcnn/include
)

# Configure built shared libraries in top-level lib directory
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# Build the shared libraries
# add_library (frvt_11_custom_001 SHARED customimplfrvt11.cpp)
add_library (frvt_11_custom_001 SHARED ${DIR_SRCS})

target_link_libraries (frvt_11_custom_001 ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libmtcnn.so ${OpenCV_LIBS})


