cmake_minimum_required(VERSION 3.22)
project(obs_headless_client)

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
# cbs-headless-client
###################

set(CMAKE_INSTALL_PREFIX "/opt/obs-headless")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
add_definitions(-DOBS_HEADLESS_PATH="${CMAKE_INSTALL_PREFIX}")

add_executable(client 
    client.cpp
    ../lib/Trace.hpp
    ../lib/studio.pb.h
    ../lib/studio.pb.cc
    ../lib/studio.grpc.pb.h
    ../lib/studio.grpc.pb.cc
)

target_link_libraries(client
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
)

install(TARGETS client
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
