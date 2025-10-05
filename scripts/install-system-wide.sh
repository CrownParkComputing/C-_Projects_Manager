#!/bin/bash

# C++ Workspace Manager - System-Wide Installation Script
# This script installs cppm system-wide

set -e

echo "🚀 Installing C++ Workspace Manager system-wide..."

# Check if executable exists
if [ ! -f "build/cppm" ]; then
    echo "❌ Error: build/cppm not found. Please build the project first."
    exit 1
fi

# Install executable to /usr/local/bin
echo "📦 Installing executable to /usr/local/bin..."
sudo cp build/cppm /usr/local/bin/
sudo chmod +x /usr/local/bin/cppm
echo "✅ Executable installed successfully"

# Create desktop entry
echo "🖥️  Creating desktop entry..."
mkdir -p ~/.local/share/applications
cat > ~/.local/share/applications/cppm.desktop << EOF
[Desktop Entry]
Name=C++ Workspace Manager
Comment=Manage C++ development workspaces
Exec=/usr/local/bin/cppm
Icon=applications-development
Terminal=false
Type=Application
Categories=Development;IDE;
EOF

# Update desktop database
if command -v update-desktop-database &> /dev/null; then
    echo "🔄 Updating desktop database..."
    update-desktop-database ~/.local/share/applications/
fi

echo ""
echo "🎉 Installation completed successfully!"
echo "✅ You can now run 'cppm' from anywhere in the terminal"
echo "✅ You can also find 'C++ Workspace Manager' in your application menu"
echo ""
echo "To uninstall, run:"
echo "  sudo rm /usr/local/bin/cppm"
echo "  rm ~/.local/share/applications/cppm.desktop"