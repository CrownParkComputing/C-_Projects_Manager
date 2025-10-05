#!/bin/bash

# C++ Workspace Manager - System-Wide Uninstallation Script
# This script removes cppm from system-wide installation

set -e

echo "🗑️  Uninstalling C++ Workspace Manager from system..."

# Remove executable from /usr/local/bin
if [ -f "/usr/local/bin/cppm" ]; then
    echo "📦 Removing executable from /usr/local/bin..."
    sudo rm /usr/local/bin/cppm
    echo "✅ Executable removed successfully"
else
    echo "ℹ️  Executable not found in /usr/local/bin"
fi

# Remove desktop entry
if [ -f ~/.local/share/applications/cppm.desktop ]; then
    echo "🖥️  Removing desktop entry..."
    rm ~/.local/share/applications/cppm.desktop
    echo "✅ Desktop entry removed successfully"
else
    echo "ℹ️  Desktop entry not found"
fi

# Update desktop database
if command -v update-desktop-database &> /dev/null; then
    echo "🔄 Updating desktop database..."
    update-desktop-database ~/.local/share/applications/
fi

echo ""
echo "🎉 Uninstallation completed successfully!"
echo "✅ C++ Workspace Manager has been removed from your system"