# CppWorkspaceManager Scripts

This directory contains useful development and automation scripts for the CppWorkspaceManager project.

## 🚀 Quick Start Scripts

### `setup.sh`
**Complete development environment setup**
- Checks and installs required dependencies (CMake, Qt5, build tools)
- Configures and builds the project
- Creates build directory structure

```bash
./scripts/setup.sh
```

### `clean-all.sh`
**Clean all build artifacts**
- Removes build directory
- Cleans temporary files and backups
- Removes Qt MOC files and core dumps

```bash
./scripts/clean-all.sh
```

## 🛠️ Development Scripts

### `install-deps.sh`
**Install optional development tools**
- Debugging tools (GDB, Valgrind)
- Static analysis (Clang-tidy, Cppcheck)
- Code formatting (Clang-format)
- Build acceleration (ccache, Ninja)
- Development environment setup

```bash
./scripts/install-deps.sh
```

### `debug.sh`
**Interactive debugging helper**
- Run with GDB debugger
- Memory checking with Valgrind
- System call tracing with strace
- Core dump generation

```bash
./scripts/debug.sh
```

## 📦 Release and Distribution

### `package.sh`
**Create distribution packages**
- Source code tarball
- Linux binary package with install/uninstall scripts
- Docker container build files
- Cross-platform build instructions

```bash
./scripts/package.sh
```

### `github-release.sh`
**Automated GitHub release workflow**
- Version management and tagging
- Changelog generation and editing
- Package building and uploading
- GitHub release creation (with gh CLI)

```bash
./scripts/github-release.sh
```

## 📋 Script Dependencies

| Script | Required Tools | Optional Tools |
|--------|---------------|----------------|
| `setup.sh` | cmake, make, g++, pkg-config | ninja |
| `install-deps.sh` | Package manager (apt/dnf/pacman) | - |
| `debug.sh` | - | gdb, valgrind, strace |
| `package.sh` | tar, cmake, make | linuxdeploy, docker |
| `github-release.sh` | git | gh (GitHub CLI), code/gedit |

## 🔧 Usage Examples

### First-time setup:
```bash
# Clone repository
git clone <repo-url>
cd CppWorkspaceManager

# Complete setup
./scripts/setup.sh

# Install development tools (optional)
./scripts/install-deps.sh
```

### Development workflow:
```bash
# Make changes to code...

# Clean and rebuild
./scripts/clean-all.sh
./scripts/setup.sh

# Debug if needed
./scripts/debug.sh

# Test and package
./scripts/package.sh
```

### Release workflow:
```bash
# Prepare and create release
./scripts/github-release.sh
```

## 🐳 Docker Usage

After running `package.sh`, you can build and run the Docker container:

```bash
# Build Docker image
cd packages
./build-docker.sh

# Run with GUI support (Linux X11)
docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v $HOME/projects:/workspace \
  cppworkspacemanager
```

## 📝 Customization

### Adding New Scripts
1. Create script in `scripts/` directory
2. Make it executable: `chmod +x scripts/your-script.sh`
3. Follow the naming convention: `action-description.sh`
4. Add documentation to this README

### Environment Variables
- `CMAKE_BUILD_TYPE`: Debug/Release (default: Release)
- `CMAKE_GENERATOR`: Unix Makefiles/Ninja (default: Unix Makefiles)
- `PARALLEL_JOBS`: Number of parallel build jobs (default: nproc)

### Script Configuration
Most scripts can be customized by editing variables at the top of each file:
- Package names and versions
- Build directories and paths
- GitHub repository URL
- Default tools and preferences

## 🆘 Troubleshooting

### Permission Denied
```bash
chmod +x scripts/*.sh
```

### Missing Dependencies
```bash
./scripts/install-deps.sh
```

### Build Failures
```bash
./scripts/clean-all.sh
./scripts/setup.sh
```

### Qt5 Not Found
- Ubuntu/Debian: `sudo apt install qtbase5-dev`
- Fedora: `sudo dnf install qt5-qtbase-devel`
- Arch: `sudo pacman -S qt5-base`

---

💡 **Tip**: All scripts are designed to be run from the project root directory and include error checking and helpful output messages.