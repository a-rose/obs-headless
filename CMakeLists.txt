cmake_minimum_required(VERSION 2.8)

project(obs_headless)

find_package(Qt5X11Extras REQUIRED)
find_package(Qt5Widgets ${FIND_MODE})

if(NOT Qt5Widgets_FOUND)
    message(FATAL_ERROR "Failed to find Qt5")
endif()


add_definitions(-std=c++11)
add_definitions(-DDL_OPENGL="${CMAKE_SOURCE_DIR}/obs-studio/build/libobs-opengl/libobs-opengl")

set(obs_include_dir ${CMAKE_SOURCE_DIR}/obs-studio/libobs/)
set(obs_lib_dir     ${CMAKE_SOURCE_DIR}/obs-studio/build/libobs/)

message(INFO " CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(INFO " obs_include_dir = ${obs_include_dir}")
message(INFO " obs_lib_dir = ${obs_lib_dir}")

include_directories("${obs_include_dir}")
link_directories("${obs_lib_dir}")

#######################################

set(obs_SOURCES
    main.cpp
    ObsWorker.cpp
    #main_toukimarche.cpp
)

set(obs_HEADERS
    trace.h
    ObsWorker.hpp
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
