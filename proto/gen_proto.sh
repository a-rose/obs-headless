#!/bin/sh

protoc -I=. --cpp_out=../src/proto/ studio.proto
protoc -I=. --grpc_out=../src/proto/ --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin studio.proto