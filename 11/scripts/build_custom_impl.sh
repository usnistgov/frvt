#!/bin/bash

root=$(pwd)

echo $"Looking for submodule built libs: "

FILE=external/msnet-mtcnn/lib/libmtcnn.so
if [ -f "$FILE" ]; then
    echo "$FILE exist [GOOD]"
    cp external/msnet-mtcnn/lib/libmtcnn.so lib/

    mkdir -p config/mtcnn_model
    cp external/msnet-mtcnn/models/*.params config/mtcnn_model
    cp external/msnet-mtcnn/models/*.json config/mtcnn_model
else 
    echo "$FILE does not exist [FAILED]"
    exit 1
fi

echo "Attempting to build custom implementation" 
cd src/customImpl
rm -rf build; mkdir -p build; cd build
# cmake ../ > /dev/null; make
cmake ../ ; make
cd $root
