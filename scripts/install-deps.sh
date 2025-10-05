#!/bin/bash

# CppWorkspaceManager - Development Dependencies Installer
# Installs optional development tools and utilities

set -e

echo "🛠️  Installing additional development dependencies..."

# Optional development tools
OPTIONAL_TOOLS="
    gdb
    valgrind
    clang-format
    clang-tidy
    doxygen
    cppcheck
    ninja-build
    ccache
    bear
"

# Code editors and IDEs
EDITORS="
    code
    vim
    emacs
"

echo "📦 Installing debugging and analysis tools..."

# Detect package manager and install
if command -v apt-get &> /dev/null; then
    echo "Using apt (Debian/Ubuntu)..."
    
    # Add LLVM repository for latest clang tools
    if [ ! -f /etc/apt/sources.list.d/llvm.list ]; then
        echo "Adding LLVM repository..."
        wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
        echo "deb http://apt.llvm.org/$(lsb_release -cs)/ llvm-toolchain-$(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/llvm.list
        sudo apt-get update
    fi
    
    sudo apt-get install -y $OPTIONAL_TOOLS
    
    # Install VS Code if not present
    if ! command -v code &> /dev/null; then
        echo "Installing VS Code..."
        wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg
        sudo install -o root -g root -m 644 packages.microsoft.gpg /etc/apt/trusted.gpg.d/
        echo "deb [arch=amd64,arm64,armhf signed-by=/etc/apt/trusted.gpg.d/packages.microsoft.gpg] https://packages.microsoft.com/repos/code stable main" | sudo tee /etc/apt/sources.list.d/vscode.list
        sudo apt-get update
        sudo apt-get install -y code
    fi

elif command -v dnf &> /dev/null; then
    echo "Using dnf (Fedora)..."
    sudo dnf install -y $OPTIONAL_TOOLS
    
    # Install VS Code
    if ! command -v code &> /dev/null; then
        sudo rpm --import https://packages.microsoft.com/keys/microsoft.asc
        echo -e "[code]\nname=Visual Studio Code\nbaseurl=https://packages.microsoft.com/yumrepos/vscode\nenabled=1\ngpgcheck=1\ngpgkey=https://packages.microsoft.com/keys/microsoft.asc" | sudo tee /etc/yum.repos.d/vscode.repo > /dev/null
        sudo dnf check-update
        sudo dnf install -y code
    fi

elif command -v pacman &> /dev/null; then
    echo "Using pacman (Arch Linux)..."
    sudo pacman -S --noconfirm $OPTIONAL_TOOLS
    
    # Install VS Code from AUR (if yay is available)
    if command -v yay &> /dev/null; then
        yay -S --noconfirm visual-studio-code-bin
    fi
fi

echo ""
echo "⚙️  Setting up development environment..."

# Create useful aliases
echo "Creating development aliases..."
cat >> ~/.bashrc << 'EOF'

# CppWorkspaceManager Development Aliases
alias cppwm-build='cd ~/Desktop/CppWorkspaceManager && ./build.sh'
alias cppwm-run='cd ~/Desktop/CppWorkspaceManager && ./run.sh'
alias cppwm-clean='cd ~/Desktop/CppWorkspaceManager && ./scripts/clean-all.sh'
alias cppwm-setup='cd ~/Desktop/CppWorkspaceManager && ./scripts/setup.sh'

EOF

# Setup Git hooks for development
if [ -d .git ]; then
    echo "Setting up Git hooks..."
    
    # Pre-commit hook for formatting
    cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Auto-format C++ files before commit
if command -v clang-format &> /dev/null; then
    for file in $(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h)$'); do
        clang-format -i "$file"
        git add "$file"
    done
fi
EOF
    chmod +x .git/hooks/pre-commit
fi

# Configure ccache if available
if command -v ccache &> /dev/null; then
    echo "Configuring ccache for faster builds..."
    ccache --set-config max_size=5G
    ccache --set-config compression=true
    export CC="ccache gcc"
    export CXX="ccache g++"
fi

echo ""
echo "✅ Development dependencies installed!"
echo ""
echo "🔧 Installed tools:"
echo "   • gdb - GNU Debugger"
echo "   • valgrind - Memory debugging"
echo "   • clang-format - Code formatting"
echo "   • clang-tidy - Static analysis"
echo "   • doxygen - Documentation generation"
echo "   • cppcheck - Static analysis"
echo "   • ninja - Fast build system"
echo "   • ccache - Compiler cache"
echo "   • bear - Compilation database generator"
echo ""
echo "📝 New aliases available (restart shell or source ~/.bashrc):"
echo "   • cppwm-build - Quick build"
echo "   • cppwm-run - Quick run"
echo "   • cppwm-clean - Clean all"
echo "   • cppwm-setup - Full setup"
echo ""