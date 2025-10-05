#!/bin/bash

# CppWorkspaceManager - Clean All Build Artifacts
# This script removes all build artifacts and temporary files

set -e

echo "🧹 Cleaning CppWorkspaceManager build artifacts..."

# Remove build directory
if [ -d "build" ]; then
    echo "🗑️  Removing build directory..."
    rm -rf build
    echo "✅ Build directory removed"
else
    echo "ℹ️  Build directory not found"
fi

# Remove any CMake cache files in root
if [ -f "CMakeCache.txt" ]; then
    echo "🗑️  Removing CMake cache..."
    rm -f CMakeCache.txt
fi

# Remove any backup files
echo "🗑️  Removing backup files..."
find . -name "*~" -type f -delete 2>/dev/null || true
find . -name "*.bak" -type f -delete 2>/dev/null || true
find . -name ".#*" -type f -delete 2>/dev/null || true

# Remove Qt moc files if any are left
find . -name "moc_*.cpp" -type f -delete 2>/dev/null || true
find . -name "*.moc" -type f -delete 2>/dev/null || true

# Remove any core dumps
find . -name "core" -type f -delete 2>/dev/null || true
find . -name "core.*" -type f -delete 2>/dev/null || true

# Remove any Qt Creator user files
find . -name "*.pro.user*" -type f -delete 2>/dev/null || true

# Remove any VS Code generated files that shouldn't be tracked
find . -path "*/.vscode/settings.json" -delete 2>/dev/null || true

# Clean workspace cache file if it exists
if [ -f "workspaces.txt" ]; then
    echo "ℹ️  Note: Keeping workspaces.txt (contains your workspace list)"
fi

echo ""
echo "✅ Clean complete!"
echo ""
echo "🔄 To rebuild from scratch:"
echo "   ./scripts/setup.sh"
echo ""