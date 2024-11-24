#!/bin/sh

protoc \
    --proto_path=. \
    --grpc_out=. \
    --cpp_out=. \
    --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin \
    studio.proto
