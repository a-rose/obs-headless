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
add_definitions(-DLIBOBS_PATH="${OBS_INSTALL_PATH}/bin/64bit/")
add_definitions(-DLIBOBS_PLUGINS_PATH="${OBS_INSTALL_PATH}/obs-plugins/64bit/")
add_definitions(-DLIBOBS_PLUGINS_DATA_PATH="${OBS_INSTALL_PATH}/data/obs-plugins/64bit/")

include_directories("${OBS_INSTALL_PATH}/include/")
link_directories("${OBS_INSTALL_PATH}/bin/64bit/")


# Obs-headless
add_definitions(-DOBS_HEADLESS_PATH="${PROJECT_SOURCE_DIR}/")

file(GLOB_RECURSE obs_SOURCES "src/*.cpp")
file(GLOB_RECURSE obs_HEADERS "src/*.hpp")

add_executable(obs_headless ${obs_SOURCES} ${obs_HEADERS})

target_link_libraries(obs_headless
    obs
    pthread
    Qt5::Widgets
    jansson
)
