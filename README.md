# C++ Projects Manager

A powerful, Qt5-based workspace management tool for C++ developers. Streamline your development workflow with intelligent build system detection, automated building, git integration, and comprehensive project management.

![C++ Projects Manager](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-5-green.svg)
![CMake](https://img.shields.io/badge/CMake-3.16+-red.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## ✨ Features

### 🎯 **Smart Workspace Management**
- **Auto-Detection**: Automatically detects CMake, Make, Ninja, and custom build systems
- **Project Organization**: Manage multiple C++ projects from a single interface
- **Workspace Persistence**: Automatically saves workspace configuration on exit
- **Build System Intelligence**: Understands your project structure and build requirements

### 🔨 **Advanced Build Management**
- **Multi-System Support**: Switch between Make and Ninja build systems with one click
- **Parallel Building**: Optimized parallel compilation with automatic core detection
- **Build Progress**: Real-time build output and progress monitoring
- **Clean Operations**: Smart cleaning of build artifacts and temporary files

### 🚀 **Executable Management**
- **Smart Detection**: Finds only project-built executables, filters out system tools
- **Quick Launch**: One-click execution of built applications
- **GUI Detection**: Identifies and handles GUI applications appropriately
- **Build Directory Focus**: Prioritizes executables from build output directories

### 🌿 **Git Integration**
- **Version Control**: Integrated git operations and version management
- **Semantic Versioning**: Automated major.minor.patch version handling
- **Tag Management**: Create and push version tags with one click
- **Branch Operations**: Switch between branches and manage releases

### ⚙️ **Developer Experience**
- **Makefile Editing**: Direct integration with code editors for build file modifications
- **Error Handling**: Clear error messages and build failure diagnostics
- **Keyboard Shortcuts**: Efficient workflow with intuitive controls
- **Cross-Platform**: Designed for Linux with Windows and macOS compatibility

## 🚀 Quick Start

### Prerequisites
- **Qt5** development libraries
- **CMake** 3.16 or later
- **C++17** compatible compiler (GCC 7+, Clang 5+)
- **Git** for version control

### Installation

#### Option 1: Automated Setup
```bash
git clone https://github.com/yourusername/C++_Projects_Manager.git
cd C++_Projects_Manager
./scripts/setup.sh
```

#### Option 2: Manual Build
```bash
# Clone repository
git clone https://github.com/yourusername/C++_Projects_Manager.git
cd C++_Projects_Manager

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run
./cppm
```

### Ubuntu/Debian Dependencies
```bash
sudo apt update
sudo apt install qtbase5-dev cmake build-essential git ninja-build
```

### Fedora Dependencies
```bash
sudo dnf install qt5-qtbase-devel cmake gcc-c++ git ninja-build
```

### Arch Linux Dependencies
```bash
sudo pacman -S qt5-base cmake gcc git ninja
```

## 📖 Usage

### Adding Workspaces
1. Click **"Add Workspace"**
2. Select your C++ project directory
3. The tool automatically detects the build system
4. Configure build preferences (Make/Ninja)

### Building Projects
1. Select workspace from the list
2. Choose build system (Make/Ninja)
3. Click **"Build"** for compilation
4. Monitor progress in real-time
5. Use **"Clean"** to remove build artifacts

### Running Applications
1. After successful build, click **"Run"**
2. The tool finds built executables automatically
3. GUI applications are launched appropriately
4. Console applications run in terminal

### Git Operations
1. Use version buttons for semantic versioning:
   - **Major**: Breaking changes (1.0.0 → 2.0.0)
   - **Minor**: New features (1.0.0 → 1.1.0)
   - **Patch**: Bug fixes (1.0.0 → 1.0.1)
2. **"Push"** sends changes to remote repository
3. **"Commit & Push"** combines commit and push operations

## 🛠️ Development

### Project Structure
```
C++_Projects_Manager/
├── src/                    # Source code
│   ├── main.cpp           # Application entry point
│   ├── MainWindow.cpp     # Main UI implementation
│   ├── Workspace.cpp      # Workspace management logic
│   └── WorkspaceManager.cpp # Workspace collection handling
├── include/               # Header files
├── scripts/              # Development automation scripts
├── build/                # Build output directory
└── CMakeLists.txt        # CMake configuration
```

### Development Scripts
- `./scripts/setup.sh` - Complete development environment setup
- `./scripts/clean-all.sh` - Remove all build artifacts
- `./scripts/debug.sh` - Interactive debugging with GDB/Valgrind
- `./scripts/install-deps.sh` - Install additional development tools
- `./scripts/package.sh` - Create distribution packages

### Building from Source
```bash
# Debug build
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Release build
mkdir build-release && cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# With Ninja
cmake -G Ninja ..
ninja
```

## 🐳 Docker Support

### Building Container
```bash
./scripts/package.sh
cd packages
./build-docker.sh
```

### Running with GUI
```bash
# Linux with X11
docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v $HOME/projects:/workspace \
  cppprojectsmanager
```

## 📦 Distribution

### Creating Packages
```bash
./scripts/package.sh
```

Generates:
- **Source tarball** for distribution
- **Linux binary package** with installers
- **Docker containers** for consistent environments
- **Cross-platform build instructions**

### GitHub Releases
```bash
./scripts/github-release.sh
```

Automates:
- Version tagging and changelog generation
- Package building and uploading
- GitHub release creation
- Documentation updates

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

### Development Workflow
1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Commit changes: `git commit -m 'Add amazing feature'`
4. Push to branch: `git push origin feature/amazing-feature`
5. Open a Pull Request

### Code Style
- Follow C++17 best practices
- Use Qt naming conventions for UI elements
- Include comprehensive error handling
- Add unit tests for new functionality

## 🐛 Troubleshooting

### Build Issues
```bash
# Clean everything and rebuild
./scripts/clean-all.sh
./scripts/setup.sh

# Check dependencies
./scripts/install-deps.sh
```

### Qt5 Not Found
```bash
# Ubuntu/Debian
sudo apt install qtbase5-dev

# Set Qt5 path manually
export Qt5_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt5
```

### Permission Errors
```bash
# Make scripts executable
chmod +x scripts/*.sh

# Fix ownership if needed
sudo chown -R $USER:$USER .
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Qt Framework** for the excellent GUI toolkit
- **CMake** for flexible build system management
- **Linux Community** for development tools and support
- **Open Source Contributors** for inspiration and code quality standards

## 📬 Contact

- **GitHub Issues**: [Report bugs and feature requests](https://github.com/yourusername/C++_Projects_Manager/issues)
- **Discussions**: [Community discussions and support](https://github.com/yourusername/C++_Projects_Manager/discussions)

---

**Made with ❤️ for C++ developers by C++ developers**