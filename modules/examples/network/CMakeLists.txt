include_directories(
    global
)

link_libraries(
    ${Qt_LIBS}
)

set(LIB_NAME network)

set(MODULESDIR ${CMAKE_INSTALL_LIBDIR}/dde-session-shell/modules)

aux_source_directory(. SRCS)

# add_library(${LIB_NAME} STATIC ${SRCS})

add_library(${LIB_NAME} SHARED ${SRCS})

install(TARGETS ${LIB_NAME} LIBRARY DESTINATION ${MODULESDIR})
