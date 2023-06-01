#!/bin/bash

set -xe

read -p "Please enter your build type (Release/Debug): " build_type

rm -rf build -DCMAKE_BUILD_TYPE=${build_type}

cmake -S . -B build

cd build

make

./WalnutApp/WalnutApp
