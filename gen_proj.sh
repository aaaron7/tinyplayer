#!/bin/bash
#########################################################################
# File Name: see_me_in_xcode.sh
# Author: aaron
# mail: aaaron7@outlook.com
# Created Time: 2019-08-04 22:38:11
#########################################################################
rm -rf build/
mkdir build
cd build/
cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=./ios.cmake -DIOS_PLATFORM=OS ../src -Wno-dev
