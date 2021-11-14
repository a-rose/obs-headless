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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Server
add_executable(obs_headless_server
    server.cpp
    Settings.cpp
    proto/studio.pb.cc
    proto/studio.grpc.pb.cc
    Studio.cpp
    Source.cpp
    Scene.cpp
    Show.cpp
    Trace.hpp
    Settings.hpp
    proto/studio.pb.h
    proto/studio.grpc.pb.h
    Studio.hpp
    Source.hpp
    Scene.hpp
    Show.hpp
)

target_link_libraries(obs_headless_server
    obs
    pthread
    Qt5::Widgets
    jansson
    protobuf
    grpc++
    grpc++_reflection
)

# Client
add_executable(obs_headless_client 
    client.cpp
    proto/studio.pb.cc
    proto/studio.grpc.pb.cc
    Trace.hpp
    proto/studio.pb.h
    proto/studio.grpc.pb.h
)

target_link_libraries(obs_headless_client
    protobuf
    grpc++
    grpc++_reflection
)
