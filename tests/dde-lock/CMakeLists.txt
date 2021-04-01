set(BIN_NAME dde-lock-test)

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)

aux_source_directory(. LOCK_TEST)

set(LOCK_TEST_SRCS
    ${LOCK_TEST}
    ${authority_DBUS_SCRS}
    ${GLOBAL_UTILS}
    ${GLOBAL_UTILS_DBUS}
    ${WIDGETS}
    ${SESSION_WIDGETS}
    ${haweiswitchos_DBUS_SCRS}
    ${AUTHENTICATE}
    ${PROJECT_SOURCE_DIR}/src/dde-lock/lockframe.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/lockworker.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus/dbuslockagent.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus/dbuslockfrontservice.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus/dbusshutdownagent.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus/dbusshutdownfrontservice.cpp
)

add_executable(${BIN_NAME}
    ${LOCK_TEST_SRCS}
    ${QRCS}
    ${LOCK_QRCS}
)

target_include_directories(${BIN_NAME} PUBLIC
    ${DTKWIDGET5.5_INCLUDE_DIR}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${QGSettings_INCLUDE_DIRS}
    ${PAM_INCLUDE_DIR}
    ${XCB_EWMH_INCLUDE_DIRS}
    ${Qt5X11Extras_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src/dde-lock
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt5Test_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget5.5_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${Qt_LIBS}
    ${PAM_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    -lpthread
    -lm
)

add_custom_target(dde-lock-check
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/tests)

add_custom_command(TARGET dde-lock-check
    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "

    #1.清理原先的gcov信息
    COMMAND lcov --directory ./CMakeFiles/${BIN_NAME}.dir --zerocounters
    COMMAND ./${BIN_NAME}

    #2.收集gcov信息到.info文件中
    COMMAND lcov --directory . --capture --output-file ./coverage.info

    #3.过滤一些我们不感兴趣的文件的覆盖率信息
    COMMAND echo " =================== do filter begin ==================== "
    COMMAND lcov --remove ./coverage.info
    '*/${BIN_NAME}_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*' '*/widgets/*' '*/interface/*'
    -o ./coverage.info
    COMMAND echo " =================== do filter end ==================== "

    #3.将.info信息生成报告到reprot文件夹中
    COMMAND genhtml -o ${CMAKE_BINARY_DIR}/tests/report ./coverage.info

    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "

    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/tests/report "

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

#'make check'命令依赖与我们的测试程序
add_dependencies(dde-lock-check ${BIN_NAME})
