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
set(CMAKE_INSTALL_PREFIX "/opt/obs-headless")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_INSTALL_RPATH "${OBS_INSTALL_PATH}/bin/64bit/")
add_definitions(-DOBS_HEADLESS_PATH="${CMAKE_INSTALL_PREFIX}")


# Server
add_executable(obs_headless_server
    server.cpp
    lib/Settings.cpp
    lib/proto/studio.pb.cc
    lib/proto/studio.grpc.pb.cc
    lib/Studio.cpp
    lib/Source.cpp
    lib/Scene.cpp
    lib/Show.cpp
    lib/Trace.hpp
    lib/Settings.hpp
    lib/proto/studio.pb.h
    lib/proto/studio.grpc.pb.h
    lib/Studio.hpp
    lib/Source.hpp
    lib/Scene.hpp
    lib/Show.hpp
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

install(TARGETS obs_headless_server
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)

# Client
add_executable(obs_headless_client 
    client.cpp
    lib/proto/studio.pb.cc
    lib/proto/studio.grpc.pb.cc
    lib/Trace.hpp
    lib/proto/studio.pb.h
    lib/proto/studio.grpc.pb.h
)

target_link_libraries(obs_headless_client
    protobuf
    grpc++
    grpc++_reflection
)

install(TARGETS obs_headless_client
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
