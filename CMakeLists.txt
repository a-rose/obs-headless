cmake_minimum_required(VERSION 2.8)
project(obs_headless)

# Qt
find_package(Qt5X11Extras REQUIRED)
find_package(Qt5Widgets ${FIND_MODE})
if(NOT Qt5Widgets_FOUND)
    message(FATAL_ERROR "Failed to find Qt5")
endif()
set(qtlibs Qt5::Widgets)

# Obs
set(obs_dir ${CMAKE_SOURCE_DIR}/../obs-studio)
set(obs_include_dir ${obs_dir}/libobs/)
set(obs_lib_dir     ${obs_dir}/build/libobs/)

add_definitions(-DDL_OPENGL="${obs_dir}/build/libobs-opengl/libobs-opengl")
add_definitions(-DROOT_DIR="${CMAKE_SOURCE_DIR}")

include_directories("${obs_include_dir}")
link_directories("${obs_lib_dir}")

#######################################

set(obs_SOURCES
    src/main.cpp
    src/ObsWorker.cpp
)

set(obs_HEADERS
    src/trace.h
    src/ObsWorker.hpp
)

######################################

add_executable(obs_headless
    ${obs_SOURCES}
    ${obs_HEADERS})

target_link_libraries(obs_headless
    obs
    pthread
    Qt5::Widgets
)
