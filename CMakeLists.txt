cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME cutefish-photos)
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

list(APPEND CMAKE_PREFIX_PATH "/usr/lib/cmake/CutefishToolkit" "/usr/local/lib/cmake/CutefishToolkit" "${CMAKE_SOURCE_DIR}/../cutefish-toolkit/build")

find_package(CutefishToolkit REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n)

set(SRCS
    src/main.cpp
    src/backend/photo_store.cpp
    src/ui/main_window.cpp
    src/ui/sidebar_view.cpp
    src/ui/header_bar.cpp
    src/ui/views/gallery_view.cpp
    src/ui/views/image_viewer.cpp
)

set(HEADERS
    src/backend/photo_store.h
    src/ui/main_window.h
    src/ui/sidebar_view.h
    src/ui/header_bar.h
    src/ui/views/gallery_view.h
    src/ui/views/image_viewer.h
)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Cutefish::Toolkit
    PkgConfig::ICU
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin)

install(FILES
    aevis-photos.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)

install(CODE "execute_process(COMMAND \${CMAKE_COMMAND} -E create_symlink ${PROJECT_NAME} \$ENV{DESTDIR}\${CMAKE_INSTALL_FULL_BINDIR}/aevis-photos)")
