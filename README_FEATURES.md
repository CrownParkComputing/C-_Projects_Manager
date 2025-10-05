# C++ Workspace Manager - Management Actions

## Features Added

### Build Management
The application now includes build management controls for selected workspaces:

- **Build**: Compiles the workspace using the built-in build system
- **Clean**: Removes build artifacts (build directory)
- **Run**: Executes the built application (placeholder for now)

### Git Versioning
Automated semantic versioning with git tags:

- **Major Version**: Increments major version (x.0.0) - breaking changes
- **Minor Version**: Increments minor version (x.y.0) - new features
- **Patch Version**: Increments patch version (x.y.z) - bug fixes

### Git Operations
- **Branch Selection**: Choose target branch (main, master, develop)
- **Push**: Push current branch with all tags to remote
- **Commit & Push**: Add all changes, commit with message, and push

## How to Use

1. **Select a Workspace**: Click on a workspace in the left panel
2. **Build Management**: Use the "Build Management" group buttons
   - Build: Compile the project
   - Clean: Remove build artifacts
   - Run: Execute the application
3. **Version Management**: Use the "Git Versioning" group
   - View current version at the top
   - Click Major/Minor/Patch to create new version tags
   - Select target branch from dropdown
   - Use Push or Commit & Push for git operations

## Version Tag Format
- Tags are created in format: `vX.Y.Z` (e.g., v1.2.3)
- Annotated tags with version message
- Automatically increments based on type selected

## Requirements
- Git repository initialized in workspace
- CMake-based project (for build functionality)
- Proper git remote configured (for push operations)

## Building the Application
```bash
cd /path/to/CppWorkspaceManager
./build.sh
```

## Running the Application
```bash
cd /path/to/CppWorkspaceManager/build
DISPLAY=:0 ./cppm
```