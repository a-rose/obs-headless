#!/bin/sh

protoc -I=. --cpp_out=../lib/proto/ studio.proto
protoc -I=. --grpc_out=../lib/proto/ --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin studio.proto