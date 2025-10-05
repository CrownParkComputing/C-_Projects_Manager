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
- **Automatic Naming**: Workspace names are automatically generated from folder or repository names
- **Workspace Removal**: Remove workspaces from list with confirmation (files preserved)
- **GitHub Integration**: Clone repositories directly into workspaces with one click

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

### 🌿 **Git & GitHub Integration**
- **Version Control**: Integrated git operations and version management
- **Semantic Versioning**: Automated major.minor.patch version handling
- **Tag Management**: Create and push version tags with one click
- **Branch Operations**: Switch between branches and manage releases
- **GitHub Authentication**: Secure authentication with Personal Access Tokens
- **Repository Browsing**: View and explore your GitHub repositories with detailed information
- **Repository Creation**: Create new repositories directly from the application
- **Repository Management**: Clone repositories and manage GitHub integration
- **Repository Browsing**: Browse and clone your GitHub repositories
- **Repository Creation**: Create new GitHub repositories directly from the app

### ⚙️ **Developer Experience**
- **Makefile Editing**: Direct integration with code editors for build file modifications
- **Error Handling**: Clear error messages and build failure diagnostics
- **Keyboard Shortcuts**: Efficient workflow with intuitive controls
- **Cross-Platform**: Designed for Linux with Windows and macOS compatibility

### 🔧 **Script Management**
- **Script Discovery**: Automatically detects and lists available scripts in the `scripts/` directory
- **One-Click Execution**: Run setup, cleanup, packaging, and other development scripts with progress monitoring
- **System-Wide Installation**: Install C++ Projects Manager system-wide with desktop integration
- **Real-Time Output**: See script execution progress and output in real-time
- **Error Handling**: Clear feedback on script success or failure with detailed output

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

#### Option 3: System-Wide Installation
After building the project:
1. Run the application: `./run.sh`
2. In the application, click **"Install System-Wide"**
3. Enter your password when prompted
4. Access from anywhere: `cppm` or find "C++ Workspace Manager" in your application menu

Or manually install:
```bash
./scripts/install-system-wide.sh
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
1. **Add Local Folder**: Click **"Add Local Folder"** and select your C++ project directory
2. **Clone from GitHub**: Click **"Clone from GitHub"** and enter a repository URL
   - Supports both HTTPS and SSH URLs
   - Automatically creates workspace with repository name
   - Select destination directory for cloning
3. The tool automatically detects the build system
4. Configure build preferences (Make/Ninja)

### Managing Workspaces
1. **Remove Workspace**: Select a workspace and click **"Remove Workspace"**
   - Only removes from the workspace list (files are preserved)
   - Confirmation dialog prevents accidental removal
2. **Refresh**: Update workspace information and build status

**Note**: Workspace names are automatically generated from folder or repository names.

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
4. **GitHub Integration**:
   - **"Rename Repo"**: Rename the local repository directory
   - **"Create Release"**: Create GitHub releases with tags and notes
   - **"Open on GitHub"**: Open the repository in your browser
   - **"View Issues"**: Open the GitHub issues page in your browser
   - **"GitHub Auth"**: Authenticate with GitHub using personal access token
   - **"Browse My Repos"**: Browse and clone your GitHub repositories
   - **"Create Repo"**: Create new repositories on GitHub

### GitHub Authentication
1. Click **"GitHub Auth"** to authenticate with GitHub
2. Enter your Personal Access Token (create one at: https://github.com/settings/tokens)
3. Required token scopes: `repo`, `user`
4. Once authenticated, you can:
   - Browse all your repositories
   - Clone repositories directly into workspaces
   - Create new repositories on GitHub

### Script Management
1. **View Available Scripts**: All `.sh` files in the `scripts/` directory are automatically detected
2. **Run Scripts**: Select a script from the dropdown and click **"Run Script"**
   - Real-time output display in the build output area
   - Progress indication during execution
   - Clear success/failure notifications
3. **System-Wide Installation**: Click **"Install System-Wide"** to:
   - Install the executable to `/usr/local/bin/cppm`
   - Create a desktop entry for easy access
   - Make the application available from anywhere in the terminal
4. **Available Scripts**:
   - `setup.sh` - Development environment setup
   - `clean-all.sh` - Clean all build artifacts
   - `install-deps.sh` - Install development dependencies
   - `package.sh` - Create distribution packages
   - `install-system-wide.sh` - System-wide installation
   - `uninstall-system-wide.sh` - Remove system-wide installation

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
- `./scripts/install-system-wide.sh` - Install application system-wide
- `./scripts/uninstall-system-wide.sh` - Remove system-wide installation

All scripts can be executed directly or through the application's Script Management interface.

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