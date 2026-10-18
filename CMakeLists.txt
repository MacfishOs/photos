cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME cutefish-photos)
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(GNUInstallDirs)

set(QT Core Gui Widgets Quick QuickControls2 DBus Concurrent LinguistTools)
find_package(Qt6 REQUIRED ${QT})

include_directories(src)

set(SRCS
    src/main.cpp
    src/photosapp.cpp
    src/photosapp.h
    src/foldermodel.cpp
    src/foldermodel.h
    src/photomodel.cpp
    src/photomodel.h
    src/imageprovider.cpp
    src/imageprovider.h
)

set(RESOURCES
    src/qml.qrc
)

file(GLOB TS_FILES translations/*.ts)
qt6_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

add_executable(${PROJECT_NAME} ${SRCS} ${RESOURCES})
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
    Qt6::Concurrent
)

add_dependencies(${PROJECT_NAME} translations)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin)

install(FILES
    aevis-photos.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)

install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)
install(FILES ${QM_FILES} DESTINATION /usr/share/aevis-photos/translations)
install(FILES src/images/cutefish-photos.svg DESTINATION /usr/share/icons/hicolor/scalable/apps)
install(FILES src/images/cutefish-photos.svg DESTINATION /usr/share/icons/hicolor/scalable/apps RENAME aevis-photos.svg)

install(CODE "execute_process(COMMAND \${CMAKE_COMMAND} -E create_symlink ${PROJECT_NAME} \$ENV{DESTDIR}\${CMAKE_INSTALL_FULL_BINDIR}/aevis-photos)")
