#!/bin/bash

set -e
set -x

mkdir -p build
pushd build
clang -c ../argparse.c ../query.c ../notes.c -fPIC
clang ../main.c argparse.o query.o notes.o -o main -lsqlite3
clang argparse.o query.o notes.o -shared -o libnotes.dylib -lsqlite3
popd
