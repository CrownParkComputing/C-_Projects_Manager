#!/bin/bash

# CppWorkspaceManager - Debug Helper Script
# Runs the application with debugging tools

set -e

echo "🐛 CppWorkspaceManager Debug Helper"

# Check if build exists
if [ ! -f "build/cppm" ]; then
    echo "❌ Application not built. Building now..."
    ./build.sh
fi

# Debug options
echo "Select debugging option:"
echo "1) Run normally"
echo "2) Run with GDB"
echo "3) Run with Valgrind (memory check)"
echo "4) Run with Valgrind (memory leaks)"
echo "5) Run with strace (system calls)"
echo "6) Generate core dump on crash"

read -p "Enter choice (1-6): " choice

case $choice in
    1)
        echo "🚀 Running normally..."
        cd build && ./cppm
        ;;
    2)
        if command -v gdb &> /dev/null; then
            echo "🔍 Running with GDB..."
            cd build && gdb -ex run ./cppm
        else
            echo "❌ GDB not installed. Install with: sudo apt install gdb"
            exit 1
        fi
        ;;
    3)
        if command -v valgrind &> /dev/null; then
            echo "🔍 Running with Valgrind (memory check)..."
            cd build && valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./cppm
        else
            echo "❌ Valgrind not installed. Install with: sudo apt install valgrind"
            exit 1
        fi
        ;;
    4)
        if command -v valgrind &> /dev/null; then
            echo "🔍 Running with Valgrind (detailed leak check)..."
            cd build && valgrind --tool=memcheck --leak-check=full --leak-resolution=high --show-leak-kinds=all ./cppm
        else
            echo "❌ Valgrind not installed. Install with: sudo apt install valgrind"
            exit 1
        fi
        ;;
    5)
        if command -v strace &> /dev/null; then
            echo "🔍 Running with strace..."
            cd build && strace -o trace.log ./cppm
            echo "System call trace saved to build/trace.log"
        else
            echo "❌ strace not installed. Install with: sudo apt install strace"
            exit 1
        fi
        ;;
    6)
        echo "🔍 Setting up core dump..."
        ulimit -c unlimited
        echo "Core dumps enabled (unlimited size)"
        echo "🚀 Running application..."
        cd build && ./cppm
        if [ -f core ]; then
            echo "💥 Core dump generated: build/core"
            echo "To analyze: gdb ./cppm core"
        fi
        ;;
    *)
        echo "❌ Invalid choice"
        exit 1
        ;;
esac

echo ""
echo "✅ Debug session completed"