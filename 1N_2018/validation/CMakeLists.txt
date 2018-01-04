cmake_minimum_required(VERSION 2.8)
project(frvt1N_validation)
set(CMAKE_BUILD_TYPE Release)

# Build libraries
#add_subdirectory(src/nullImpl)

# Build test driver
add_subdirectory(src/testdriver)
