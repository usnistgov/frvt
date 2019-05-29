#!/bin/bash

root=$(pwd)

echo "Attempting to build null implementation" 
cd src/nullImpl
rm -rf build; mkdir -p build; cd build
cmake ../ > /dev/null; make
cd $root
