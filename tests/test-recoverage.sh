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

lcov -c -i -d ./ -o init.info

./dde-lock/dde-lock-test
lcov -c -d ./ -o dde-lock.info

./lightdm-deepin-greeter/lightdm-deepin-greeter-test
lcov -c -d ./ -o lightdm-deepin-greeter.info

lcov -a init.info -a dde-lock.info -a lightdm-deepin-greeter.info -o total.info
lcov -r total.info "*/tests/*" "*/usr/include*"  "*.h" "*build/*" "*/dbus/*" "*/xcb/*" -o final.info

rm -rf ../../tests/$REPORT_DIR
mkdir -p ../../tests/$REPORT_DIR
genhtml -o ../../tests/$REPORT_DIR final.info
