#!/bin/sh

# organize folders
cd ..
rm -rf release
mkdir -p release/example
cp -rf src/pcsx2_ipc.h release/
cp -rf windows-qt.pro meson.build src/ release/example/
cp -rf bindings/ release/

# generate docs
doxygen
mkdir -p release/docs
cp -rf html release/docs
cd latex && make
cd ..
cp -rf latex/refman.pdf release/docs

# remove build artifacts
find release -type d -name build -prune -exec rm -rf {} \;
find release -type d -name bin -prune -exec rm -rf {} \;
find release -type d -name obj -prune -exec rm -rf {} \;
find release -type d -name libpcsx2_ipc_c.so -prune -exec rm -rf {} \;
find release -type d -name target -prune -exec rm -rf {} \;

# test cases, to see if we've broken something between releases
# and code coverage because why not :D
meson build -Db_coverage=true
cd build
# pcsx2 takes time to start up D:
meson test --timeout-multiplier=10
ninja coverage-html
mkdir -p release/tests
cp -rf meson-logs/coveragereport/ ../release/tests
python ../utils/pretty-tests.py meson-logs/testlog.json > release/tests/result.txt
cd ..


# make the release zip
zip -r release.zip release &> /dev/null
