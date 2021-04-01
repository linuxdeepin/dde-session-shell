set(BIN_NAME lightdm-deepin-greeter-test)

# 用于测试覆盖率的编译条件
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)

aux_source_directory(. GREETER_TEST)

set(GREETER_TEST_SRCS
    ${GREETER_TEST}
    ${authority_DBUS_SCRS}
    ${transaction_DBUS_SCRS}
    ${GLOBAL_UTILS}
    ${GLOBAL_UTILS_DBUS}
    ${WIDGETS}
    ${SESSION_WIDGETS}
    ${haweiswitchos_DBUS_SCRS}
    ${AUTHENTICATE}
    ${PROJECT_SOURCE_DIR}/src/lightdm-deepin-greeter/loginwindow.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-deepin-greeter/greeterworkek.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-deepin-greeter/logincontent.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-deepin-greeter/logintipswindow.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-deepin-greeter/sessionwidget.cpp
)

add_executable(${BIN_NAME}
    ${GREETER_TEST_SRCS}
    ${QRCS}
    ${GREETER_QRCS}
)

target_include_directories(${BIN_NAME} PUBLIC
    ${PAM_INCLUDE_DIR}
    ${DTKWIDGET5.5_INCLUDE_DIR}
    ${XCB_EWMH_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ${QGSettings_INCLUDE_DIRS}
    ${Qt5X11Extras_INCLUDE_DIRS}
    ${Greeter_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/lightdm-deepin-greeter
)

target_compile_definitions(lightdm-deepin-greeter
    PRIVATE ENABLE_SESSION
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${PAM_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget5.5_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${Greeter_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${GTEST_LIBRARIES}
    -lpthread
    -lm
)

add_custom_target(lightdm-deepin-greeter-check
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/tests)

add_custom_command(TARGET lightdm-deepin-greeter-check
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
add_dependencies(lightdm-deepin-greeter-check ${BIN_NAME})
