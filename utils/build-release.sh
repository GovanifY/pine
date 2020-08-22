#!/bin/sh
cd .
mkdir release
cp -rf pcsx2_ipc.h release/
mkdir -p release/example
cp -rf Makefile windows-qt.pro client.cpp pcsx2_ipc.h release/example/
doxygen
mkdir -p release/docs
cp -rf html release/docs
cd latex && make
cd ..
cp -rf latex/refman.pdf release/docs
zip -r release.zip release
