find_package(Qt5 COMPONENTS Test DBus REQUIRED)
find_package(GTest REQUIRED)

add_subdirectory(dde-lock)
add_subdirectory(lightdm-deepin-greeter)
