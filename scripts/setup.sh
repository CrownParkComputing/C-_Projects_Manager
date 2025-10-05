#!/bin/bash

# CppWorkspaceManager - Development Setup Script
# This script sets up the development environment

set -e  # Exit on any error

echo "🚀 Setting up CppWorkspaceManager development environment..."

# Check for required tools
echo "📋 Checking dependencies..."

check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo "❌ $1 is not installed"
        return 1
    else
        echo "✅ $1 is available"
        return 0
    fi
}

# Essential build tools
MISSING_DEPS=""

if ! check_command "cmake"; then
    MISSING_DEPS="$MISSING_DEPS cmake"
fi

if ! check_command "make"; then
    MISSING_DEPS="$MISSING_DEPS make"
fi

if ! check_command "g++"; then
    MISSING_DEPS="$MISSING_DEPS g++"
fi

if ! check_command "pkg-config"; then
    MISSING_DEPS="$MISSING_DEPS pkg-config"
fi

# Check for Qt5
echo "🔍 Checking for Qt5..."
if ! pkg-config --exists Qt5Core Qt5Widgets Qt5Gui; then
    echo "❌ Qt5 development libraries not found"
    MISSING_DEPS="$MISSING_DEPS qtbase5-dev"
else
    echo "✅ Qt5 development libraries found"
fi

# Optional tools
if ! check_command "ninja"; then
    echo "⚠️  Ninja build system not found (optional but recommended)"
    MISSING_DEPS="$MISSING_DEPS ninja-build"
fi

if ! check_command "git"; then
    echo "❌ Git is required for version control"
    MISSING_DEPS="$MISSING_DEPS git"
fi

# Install missing dependencies
if [ -n "$MISSING_DEPS" ]; then
    echo ""
    echo "📦 Installing missing dependencies..."
    echo "Required packages: $MISSING_DEPS"
    
    # Detect package manager
    if command -v apt-get &> /dev/null; then
        echo "Using apt (Debian/Ubuntu)..."
        sudo apt-get update
        sudo apt-get install -y $MISSING_DEPS
    elif command -v dnf &> /dev/null; then
        echo "Using dnf (Fedora)..."
        sudo dnf install -y $MISSING_DEPS
    elif command -v pacman &> /dev/null; then
        echo "Using pacman (Arch Linux)..."
        sudo pacman -S --noconfirm $MISSING_DEPS
    else
        echo "❌ Could not detect package manager. Please install manually:"
        echo "   $MISSING_DEPS"
        exit 1
    fi
fi

echo ""
echo "🏗️  Setting up build directory..."
mkdir -p build
cd build

echo "⚙️  Configuring with CMake..."
cmake ..

echo "🔨 Building project..."
make -j$(nproc)

echo ""
echo "✅ Setup complete!"
echo ""
echo "📁 Project structure:"
echo "   build/           - Build output directory"
echo "   build/cppm       - Main executable"
echo "   scripts/         - Development scripts"
echo ""
echo "🚀 To run the application:"
echo "   cd build && ./cppm"
echo ""
echo "🔄 To rebuild:"
echo "   cd build && make"
echo ""
echo "🧹 To clean:"
echo "   ./scripts/clean-all.sh"
echo ""