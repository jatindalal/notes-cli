#!/bin/bash

set -e
set -x

rustc ./main.rs -o main -L ../build -l notes --edition=2024
