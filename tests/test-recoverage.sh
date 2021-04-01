#!/bin/bash
BUILD_DIR=build
REPORT_DIR=report
cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ../
make -j 16
cd tests/
./dde-lock/dde-lock-test
./lightdm-deepin-greeter/lightdm-deepin-greeter-test
lcov -c -d ./ -o coverage.info
lcov -r coverage.info "*/tests/*" "*/usr/include*"  "*.h" "*build/*" "*/dbus/*" "*/xcb/*" -o final.info
rm -rf ../../tests/$REPORT_DIR
mkdir -p ../../tests/$REPORT_DIR
genhtml -o ../../tests/$REPORT_DIR final.info
