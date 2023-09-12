#!/bin/bash
# Build script for testing

# Get a list of all the .c files
cFilenames=$(find . -type -f -name "*.c")

# echo "Files:" $cFilenames

assembly="tests"
compilerFlags="-g -fdeclspec -fPIC"
# -fms-extensions
# -Wall -Werror 

includeFlags="-Isrc -I../engine/src/"
linkerFlags="-L../bin/ -lengine -Wl, -rpath,."
defines="-D_DEBUG -DLIMPORT"

ECHO "Building $assembly..."
clang $cFilenames $compilerFlags -o ../bin/$assembly $defines $includeFlags $linkerFlags