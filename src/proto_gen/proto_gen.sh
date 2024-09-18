#!/bin/sh

protoc \
    --proto_path=. \
    --grpc_out=../lib/proto/ \
    --cpp_out=../lib/proto/ \
    --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin \
    studio.proto
