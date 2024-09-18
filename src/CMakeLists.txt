cmake_minimum_required(VERSION 3.22)
project(obs_headless)


###################
# Qt
###################

find_package(Qt6Widgets ${FIND_MODE})
if(NOT Qt6Widgets_FOUND)
    message(FATAL_ERROR "Failed to find Qt6Widgets_FOUND")
else()
    message(STATUS "Using Qt6Widgets ${Qt6Widgets_VERSION}")
endif()

set(qtlibs Qt6::Widgets)
find_package(Qt6Gui)
if(NOT Qt6Gui_FOUND)
    message(FATAL_ERROR "Failed to find Qt6Gui")
else()
    message(STATUS "Using Qt6Gui ${Qt6Gui_VERSION}")
endif()
include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})


###################
# gRPC
###################

find_package(Protobuf CONFIG REQUIRED)
if(NOT Protobuf_FOUND)
    message(FATAL_ERROR "Failed to find Protobuf_FOUND")
else()
    message(STATUS "Using Protobuf ${Protobuf_VERSION}")
endif()

find_package(gRPC CONFIG REQUIRED)
if(NOT gRPC_FOUND)
    message(FATAL_ERROR "Failed to find gRPC_FOUND")
else()
    message(STATUS "Using gRPC ${gRPC_VERSION}")
endif()


###################
# Obs
###################

add_definitions(-DLIBOBS_PATH="${OBS_INSTALL_PATH}/bin/64bit/")
add_definitions(-DLIBOBS_PLUGINS_PATH="${OBS_INSTALL_PATH}/obs-plugins/64bit/")
add_definitions(-DLIBOBS_PLUGINS_DATA_PATH="${OBS_INSTALL_PATH}/data/obs-plugins/64bit/")

include_directories("${OBS_INSTALL_PATH}/include/")
link_directories("${OBS_INSTALL_PATH}/bin/64bit/")


###################
# Obs-headless
###################

set(CMAKE_INSTALL_PREFIX "/opt/obs-headless")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_INSTALL_RPATH "${OBS_INSTALL_PATH}/bin/64bit/")
add_definitions(-DOBS_HEADLESS_PATH="${CMAKE_INSTALL_PREFIX}")


###################
# Server
###################

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

include_directories("/include")


#   absl::check
#   absl::flags
#   absl::flags_parse
#   absl::log

target_link_libraries(obs_headless_server
    obs
    pthread
    Qt6::Widgets
    jansson
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

install(TARGETS obs_headless_server
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)


###################
# Client
###################

add_executable(obs_headless_client 
    client.cpp
    lib/proto/studio.pb.cc
    lib/proto/studio.grpc.pb.cc
    lib/Trace.hpp
    lib/proto/studio.pb.h
    lib/proto/studio.grpc.pb.h
)

target_link_libraries(obs_headless_client
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

install(TARGETS obs_headless_client
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
