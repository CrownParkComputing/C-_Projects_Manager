#!/bin/bash
# Build script for CppWorkspaceManager

set -e  # Exit on any error

echo "Building CppWorkspaceManager..."

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Run cmake configuration
echo "Running cmake configuration..."
cmake ..

# Build the project
echo "Building the project..."
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable: $(pwd)/cppm"
echo "To run the application: ./run.sh"