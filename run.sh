#!/bin/bash
# Run script for CppWorkspaceManager

# Navigate to the project directory
cd "$(dirname "$0")"

# Check if the executable exists
if [ ! -f "build/cppm" ]; then
    echo "Executable not found. Building the project first..."
    ./build.sh
fi

# Set display environment variable and run the application
echo "Starting CppWorkspaceManager..."
cd build
DISPLAY=:0 ./cppm