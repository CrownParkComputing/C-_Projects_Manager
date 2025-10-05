# Changelog

All notable changes to C++ Projects Manager will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-05

### Added
- **Workspace Management System**
  - Add and manage multiple C++ project workspaces
  - Automatic workspace persistence on application exit
  - Smart workspace detection with project information display

- **Build System Intelligence**
  - Automatic detection of CMake, Make, Ninja, and custom build systems
  - Build system selection (Make/Ninja) with UI controls
  - Intelligent build directory detection and management
  - Real-time build progress monitoring with output display

- **Advanced Executable Detection**
  - Smart filtering of project-built executables vs system tools
  - ELF binary detection for proper executable identification
  - Prioritized search in build output directories
  - GUI application detection and appropriate launching

- **Git Integration**
  - Semantic versioning with major/minor/patch controls
  - Automated git tagging and version management
  - One-click push operations for code deployment
  - Branch management and commit operations

- **User Interface**
  - Clean, intuitive Qt5-based interface
  - Workspace list with detailed project information
  - Build management panel with progress tracking
  - Git versioning controls with current version display

- **Development Tools**
  - Complete script collection for development workflow
  - Setup script for automated environment configuration
  - Debug script with GDB, Valgrind, and strace integration
  - Clean script for build artifact management

- **Distribution and Packaging**
  - Multi-platform package creation (Linux, Windows, macOS)
  - Docker container support with GUI forwarding
  - GitHub release automation with changelog management
  - Source and binary distribution packages

- **Developer Experience**
  - Makefile editing integration with external editors
  - Comprehensive error handling and user feedback
  - Build system switching without project reconfiguration
  - Development dependency installation automation

### Technical Features
- **C++17** standard with modern language features
- **Qt5** integration with Widgets, Core, and GUI modules
- **CMake** build system with automatic MOC handling
- **Cross-platform** compatibility (Linux primary, Windows/macOS support)
- **Memory safety** with proper resource management
- **Process management** for external tool integration

### Build Systems Supported
- **CMake** with generator selection (Make/Ninja)
- **GNU Make** with parallel building
- **Ninja** for fast incremental builds
- **Custom build scripts** with intelligent detection

### Scripts and Automation
- `setup.sh` - Complete development environment setup
- `clean-all.sh` - Remove all build artifacts and temporary files
- `debug.sh` - Interactive debugging with multiple tool options
- `install-deps.sh` - Install optional development tools
- `package.sh` - Create distribution packages
- `github-release.sh` - Automated release workflow

### Documentation
- Comprehensive README with usage examples
- MIT License for open source distribution
- Script documentation with troubleshooting guides
- Development workflow and contribution guidelines

---

## Release Notes

### Initial Release Highlights
This is the initial release of C++ Projects Manager, providing a complete workspace management solution for C++ developers. The application focuses on simplifying the development workflow while providing powerful features for project management, building, and distribution.

Key design principles:
- **Simplicity**: Intuitive interface for common development tasks
- **Intelligence**: Automatic detection and smart defaults
- **Flexibility**: Support for multiple build systems and workflows
- **Productivity**: Streamlined operations for efficient development

### Compatibility
- **Operating Systems**: Linux (primary), Windows, macOS
- **Compilers**: GCC 7+, Clang 5+, MSVC 2019+
- **Qt Version**: Qt5.12 or later
- **CMake Version**: 3.16 or later

### Known Limitations
- Windows and macOS support requires manual Qt5 setup
- Some features require specific development tools (git, build systems)
- GUI applications may need display configuration in containerized environments

### Future Roadmap
- Cross-platform installer packages
- Project template system
- Integrated code editor capabilities
- Plugin system for extensibility
- CI/CD integration features