#!/bin/sh
# make clean, make操作

mkdir Debug
cd Debug
rm -rf *
cd ..
sh build.sh


cp test_build.sh ./Debug/bin/build.sh
cp test_clearBuild.sh ./Debug/bin/clearBuild.sh