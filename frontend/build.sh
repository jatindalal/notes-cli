#!/bin/bash

set -e
set -x

rustc ./main.rs -o main -L ../build -l notes --edition=2024 --extern gpui=./deps/target/debug/deps/libgpui-cc48e2dd8ed0f713.rlib -L ./deps/target/debug/deps/

