find_package(QT NAMES Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)


pkg_check_modules(SSL REQUIRED IMPORTED_TARGET libcrypto libssl openssl)

link_libraries(
        ${Qt_LIBS}
        Qt${QT_VERSION_MAJOR}::Widgets
        PkgConfig::SSL
        ${DtkCore_LIBRARIES}
        ${DtkWidget_LIBRARIES}
)

set(LIB_NAME assist-login)

set(LIB_NAME_INTERface assist_Login_interface)

aux_source_directory(. SRCS)

add_library(${LIB_NAME} SHARED ${SRCS})

add_subdirectory(interface)
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib/dde-session-shell/modules)