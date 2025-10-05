#!/bin/bash

# CppWorkspaceManager - Package Script
# Creates distribution packages for different platforms

set -e

VERSION=$(grep -r "VERSION" CMakeLists.txt | head -1 | sed 's/.*VERSION \([0-9.]*\).*/\1/' || echo "1.0.0")
PROJECT_NAME="CppWorkspaceManager"
BUILD_DIR="build"
PACKAGE_DIR="packages"

echo "📦 Creating packages for $PROJECT_NAME v$VERSION..."

# Clean and create package directory
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"

# Build the project first
echo "🔨 Building project..."
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake ..
    cd ..
fi

cd "$BUILD_DIR"
make -j$(nproc)
cd ..

echo "📁 Creating source package..."
# Create source tarball
tar -czf "$PACKAGE_DIR/${PROJECT_NAME}-${VERSION}-source.tar.gz" \
    --exclude=build \
    --exclude=packages \
    --exclude=.git \
    --exclude='*.user*' \
    --exclude='*~' \
    --exclude='*.bak' \
    .

echo "🐧 Creating Linux binary package..."
# Create Linux binary package
mkdir -p "$PACKAGE_DIR/linux-x64"
cp "$BUILD_DIR/cppm" "$PACKAGE_DIR/linux-x64/"
cp README.md "$PACKAGE_DIR/linux-x64/" 2>/dev/null || true
cp LICENSE "$PACKAGE_DIR/linux-x64/" 2>/dev/null || true

# Create install script for Linux
cat > "$PACKAGE_DIR/linux-x64/install.sh" << 'EOF'
#!/bin/bash
echo "Installing CppWorkspaceManager..."
sudo cp cppm /usr/local/bin/
sudo chmod +x /usr/local/bin/cppm
echo "✅ CppWorkspaceManager installed to /usr/local/bin/cppm"
echo "Run 'cppm' to start the application"
EOF
chmod +x "$PACKAGE_DIR/linux-x64/install.sh"

# Create uninstall script
cat > "$PACKAGE_DIR/linux-x64/uninstall.sh" << 'EOF'
#!/bin/bash
echo "Uninstalling CppWorkspaceManager..."
sudo rm -f /usr/local/bin/cppm
echo "✅ CppWorkspaceManager uninstalled"
EOF
chmod +x "$PACKAGE_DIR/linux-x64/uninstall.sh"

# Create archive
cd "$PACKAGE_DIR"
tar -czf "${PROJECT_NAME}-${VERSION}-linux-x64.tar.gz" linux-x64/
rm -rf linux-x64/
cd ..

echo "📱 Creating AppImage (if linuxdeploy available)..."
if command -v linuxdeploy &> /dev/null; then
    mkdir -p "$PACKAGE_DIR/appimage"
    cp "$BUILD_DIR/cppm" "$PACKAGE_DIR/appimage/"
    
    # Create desktop file
    cat > "$PACKAGE_DIR/appimage/cppm.desktop" << EOF
[Desktop Entry]
Type=Application
Name=C++ Workspace Manager
Comment=Manage and build C++ projects
Exec=cppm
Icon=cppm
Categories=Development;IDE;
EOF

    # Create AppImage (simplified, would need proper icon and dependencies)
    echo "ℹ️  AppImage creation requires additional setup - skipping for now"
else
    echo "⚠️  linuxdeploy not found - skipping AppImage creation"
fi

echo "🪟 Creating Windows build instructions..."
cat > "$PACKAGE_DIR/BUILD-WINDOWS.md" << 'EOF'
# Building CppWorkspaceManager on Windows

## Requirements
- Visual Studio 2019 or later with C++ tools
- Qt5 for Windows
- CMake 3.16 or later
- Git for Windows

## Build Steps
1. Install Qt5 from https://www.qt.io/download
2. Open "Developer Command Prompt for VS 2019"
3. Clone the repository: `git clone <repo-url>`
4. Create build directory: `mkdir build && cd build`
5. Configure: `cmake .. -DQt5_DIR=C:\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5`
6. Build: `cmake --build . --config Release`

## Package Creation
- Use NSIS or Inno Setup to create Windows installer
- Include Qt5 runtime libraries in the package
EOF

echo "🍎 Creating macOS build instructions..."
cat > "$PACKAGE_DIR/BUILD-MACOS.md" << 'EOF'
# Building CppWorkspaceManager on macOS

## Requirements
- Xcode Command Line Tools
- Qt5 (via Homebrew recommended)
- CMake

## Setup
```bash
# Install dependencies via Homebrew
brew install qt5 cmake

# Export Qt5 path
export Qt5_DIR=/usr/local/opt/qt5/lib/cmake/Qt5
```

## Build Steps
```bash
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

## Create macOS Bundle
```bash
# Create app bundle structure
mkdir -p CppWorkspaceManager.app/Contents/MacOS
cp cppm CppWorkspaceManager.app/Contents/MacOS/

# Create Info.plist
cat > CppWorkspaceManager.app/Contents/Info.plist << 'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>cppm</string>
    <key>CFBundleIdentifier</key>
    <string>com.example.cppworkspacemanager</string>
    <key>CFBundleName</key>
    <string>C++ Workspace Manager</string>
    <key>CFBundleVersion</key>
    <string>1.0.0</string>
</dict>
</plist>
PLIST

# Deploy Qt frameworks
macdeployqt CppWorkspaceManager.app
```
EOF

echo "🐳 Creating Docker build..."
cat > "$PACKAGE_DIR/Dockerfile" << 'EOF'
# Multi-stage build for CppWorkspaceManager
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    qtbase5-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /src
COPY . .

# Build
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Runtime image
FROM ubuntu:22.04

# Install Qt5 runtime
RUN apt-get update && apt-get install -y \
    libqt5widgets5 \
    libqt5core5a \
    libqt5gui5 \
    && rm -rf /var/lib/apt/lists/*

# Copy built application
COPY --from=builder /src/build/cppm /usr/local/bin/

# Create user for running the app
RUN useradd -m -s /bin/bash cppwm

USER cppwm
WORKDIR /home/cppwm

# Set up X11 forwarding for GUI
ENV DISPLAY=:0

CMD ["cppm"]
EOF

# Create Docker build script
cat > "$PACKAGE_DIR/build-docker.sh" << 'EOF'
#!/bin/bash
echo "🐳 Building CppWorkspaceManager Docker image..."
docker build -t cppworkspacemanager .

echo "✅ Docker image built successfully!"
echo ""
echo "To run with GUI (Linux with X11):"
echo "  docker run -it --rm -e DISPLAY=\$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix cppworkspacemanager"
echo ""
echo "To run and mount a workspace directory:"
echo "  docker run -it --rm -e DISPLAY=\$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v \$HOME/projects:/workspace cppworkspacemanager"
EOF
chmod +x "$PACKAGE_DIR/build-docker.sh"

echo ""
echo "✅ Package creation complete!"
echo ""
echo "📦 Created packages:"
echo "   • ${PROJECT_NAME}-${VERSION}-source.tar.gz - Source code"
echo "   • ${PROJECT_NAME}-${VERSION}-linux-x64.tar.gz - Linux binary"
echo "   • BUILD-WINDOWS.md - Windows build instructions"
echo "   • BUILD-MACOS.md - macOS build instructions"
echo "   • Dockerfile - Docker container build"
echo ""
echo "📁 All packages are in the 'packages' directory"
echo ""