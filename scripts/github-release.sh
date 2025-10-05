#!/bin/bash

# CppWorkspaceManager - GitHub Release Helper
# Automates the process of creating GitHub releases

set -e

# Configuration
REPO_URL="https://github.com/yourusername/CppWorkspaceManager"  # Update this
CHANGELOG_FILE="CHANGELOG.md"

echo "🚀 GitHub Release Helper for CppWorkspaceManager"

# Check if we're in a git repository
if [ ! -d .git ]; then
    echo "❌ This script must be run from the root of a git repository"
    exit 1
fi

# Check for uncommitted changes
if ! git diff-index --quiet HEAD --; then
    echo "⚠️  Warning: You have uncommitted changes."
    read -p "Do you want to continue? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 1
    fi
fi

# Get current version from git tags or ask user
CURRENT_VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "")
if [ -z "$CURRENT_VERSION" ]; then
    echo "📋 No previous tags found."
    read -p "Enter the version for this release (e.g., v1.0.0): " VERSION
else
    echo "📋 Current version: $CURRENT_VERSION"
    read -p "Enter the new version (e.g., v1.1.0): " VERSION
fi

# Validate version format
if [[ ! $VERSION =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "❌ Invalid version format. Use vX.Y.Z (e.g., v1.0.0)"
    exit 1
fi

echo "🏷️  Preparing release $VERSION..."

# Update version in CMakeLists.txt
if [ -f CMakeLists.txt ]; then
    VERSION_NUMBER=${VERSION#v}  # Remove 'v' prefix
    sed -i "s/VERSION [0-9]\+\.[0-9]\+\.[0-9]\+/VERSION $VERSION_NUMBER/" CMakeLists.txt
    echo "✅ Updated version in CMakeLists.txt"
fi

# Generate changelog entry
echo "📝 Generating changelog..."
if [ ! -f "$CHANGELOG_FILE" ]; then
    cat > "$CHANGELOG_FILE" << EOF
# Changelog

All notable changes to this project will be documented in this file.

## [$VERSION] - $(date +%Y-%m-%d)

### Added
- Initial release
- C++ Workspace Manager with build system detection
- Support for CMake, Make, and Ninja build systems
- Git integration with version tagging
- Executable detection and running
- Build progress monitoring

### Changed

### Fixed

EOF
else
    # Insert new version entry at the top
    temp_file=$(mktemp)
    echo "## [$VERSION] - $(date +%Y-%m-%d)" > "$temp_file"
    echo "" >> "$temp_file"
    echo "### Added" >> "$temp_file"
    echo "" >> "$temp_file"
    echo "### Changed" >> "$temp_file"
    echo "" >> "$temp_file"
    echo "### Fixed" >> "$temp_file"
    echo "" >> "$temp_file"
    
    # Insert after the first line (# Changelog)
    head -n 3 "$CHANGELOG_FILE" >> "$temp_file"
    echo "" >> "$temp_file"
    tail -n +4 "$CHANGELOG_FILE" >> "$temp_file"
    mv "$temp_file" "$CHANGELOG_FILE"
fi

# Open changelog for editing
echo "📝 Opening changelog for editing..."
if command -v code &> /dev/null; then
    code "$CHANGELOG_FILE"
elif command -v gedit &> /dev/null; then
    gedit "$CHANGELOG_FILE" &
else
    echo "Please edit $CHANGELOG_FILE manually and press Enter when done"
    read -p "Press Enter to continue..."
fi

read -p "Have you finished editing the changelog? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Please edit the changelog and run this script again."
    exit 1
fi

# Build packages
echo "📦 Building packages..."
./scripts/package.sh

# Commit changes
echo "💾 Committing changes..."
git add .
git commit -m "Release $VERSION

- Updated version to $VERSION
- Updated changelog
- Created release packages"

# Create and push tag
echo "🏷️  Creating tag..."
git tag -a "$VERSION" -m "Release $VERSION"

echo "⬆️  Pushing changes and tag..."
git push origin main
git push origin "$VERSION"

# Create GitHub release (if gh CLI is available)
if command -v gh &> /dev/null; then
    echo "🚀 Creating GitHub release..."
    
    # Extract changelog for this version
    RELEASE_NOTES=$(awk "/## \[$VERSION\]/,/## \[/{if(/## \[/ && !/## \[$VERSION\]/) exit; print}" "$CHANGELOG_FILE" | tail -n +2 | head -n -1)
    
    # Create release with packages
    gh release create "$VERSION" \
        --title "Release $VERSION" \
        --notes "$RELEASE_NOTES" \
        packages/*.tar.gz \
        packages/*.md \
        packages/Dockerfile \
        packages/*.sh
    
    echo "✅ GitHub release created successfully!"
    echo "🌐 View at: $REPO_URL/releases/tag/$VERSION"
else
    echo "ℹ️  GitHub CLI not found. Please create the release manually:"
    echo "   1. Go to $REPO_URL/releases/new"
    echo "   2. Use tag: $VERSION"
    echo "   3. Upload files from packages/ directory"
    echo "   4. Copy changelog content for release notes"
fi

echo ""
echo "🎉 Release $VERSION completed!"
echo ""
echo "📋 Summary:"
echo "   • Version: $VERSION"
echo "   • Packages created in packages/ directory"
echo "   • Git tag created and pushed"
echo "   • Changelog updated"
if command -v gh &> /dev/null; then
    echo "   • GitHub release created"
fi
echo ""
echo "🔗 Next steps:"
echo "   • Test the release packages"
echo "   • Announce the release"
echo "   • Update documentation if needed"
echo ""