find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets WebEngineWidgets REQUIRED)

include_directories(
    ${DFrameworkDBus_INCLUDE_DIRS}
    global
)

link_libraries(
    ${Qt_LIBS}
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
)

set(LIB_NAME login)

aux_source_directory(. SRCS)

# add_library(${LIB_NAME} STATIC ${SRCS})

add_library(${LIB_NAME} SHARED ${SRCS})

install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib/dde-session-shell/modules)
