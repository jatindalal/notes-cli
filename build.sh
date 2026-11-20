#!/bin/bash

gcc main.c argparse.c query.c notes.c -o main -l sqlite3
