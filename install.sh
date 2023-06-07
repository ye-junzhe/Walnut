#!/bin/bash

# read -p "Please enter your build type (Release/Debug): " build_type

read -p "Build from source? (y/n): " build_from_source


if [[ $build_from_source == [Yy] ]]; then
    echo "Building from ground up."
    rm -rf build
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
else
    echo "Building only WalnutApp"
    cmake --build ./build --target WalnutApp
fi


cd build || exit

make

./WalnutApp/WalnutApp
