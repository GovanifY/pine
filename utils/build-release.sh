#!/bin/sh
cd ..
rm -rf release
mkdir release
cp -rf src/pcsx2_ipc.h release/
mkdir -p release/example
cp -rf windows-qt.pro meson.build src/ release/example/
cp -rf bindings/ release/
doxygen
mkdir -p release/docs
cp -rf html release/docs
cd latex && make
cd ..
cp -rf latex/refman.pdf release/docs
find release -type d -name build -prune -exec rm -rf {} \;
find release -type d -name bin -prune -exec rm -rf {} \;
find release -type d -name obj -prune -exec rm -rf {} \;
zip -r release.zip release
