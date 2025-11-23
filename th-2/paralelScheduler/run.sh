#!/bin/bash
# make+run.sh
# Build the library and main program, then run the program

set -e  # stop on any error

echo "Building shared library and main program..."
make

echo "Setting LD_LIBRARY_PATH to current directory..."
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

echo "Running main program..."
./main

echo "Deleteing executables and object files"
make clean

