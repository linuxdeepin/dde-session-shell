#!/bin/bash
BUILD_DIR=build
REPORT_DIR=report
PROJECT_REALNAME=dde-session-shell

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR

cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake ../
make -j16

cd tests/
./lightdm-deepin-greeter/lightdm-deepin-greeter-test && ./dde-lock/dde-lock-test

lcov -c -d ./ -o coverage.info
lcov -r coverage.info "*/tests/*" "*/usr/include*"  "*.h" "*build/*" "*/dbus/*" "*/xcb/*" -o final.info
rm -rf ../../tests/$REPORT_DIR
mkdir -p ../../tests/$REPORT_DIR
genhtml -o ../../tests/$REPORT_DIR final.info

mv asan.log* asan_${PROJECT_REALNAME}.log
