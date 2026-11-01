#!/bin/bash

clang -I /opt/homebrew/opt/sqlite3/include/ -L /opt/homebrew/opt/sqlite3/lib/ -l sqlite3 main.c -o main
