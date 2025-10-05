#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <string>
#include <filesystem>
#include <vector>

enum class BuildSystem {
    None,
    CMake,
    Makefile,
    Ninja,
    AutoTools,
    Script  // For build.sh, build.py, etc.
};

struct ExecutableInfo {
    std::string name;
    std::string path;
    std::string relativePath;
    bool isGUI;
};

class Workspace {
public:
    Workspace(const std::string& path);
    ~Workspace() = default;

    std::string getPath() const;
    bool exists() const;

    // Build system detection
    BuildSystem detectBuildSystem() const;
    std::string getBuildSystemName() const;
    std::vector<std::string> getBuildScripts() const;
    
    // Executable detection
    std::vector<ExecutableInfo> findExecutables() const;
    ExecutableInfo findMainExecutable() const;

    // Git operations
    bool gitInit();
    bool gitAdd();
    bool gitCommit(const std::string& message);

    // Build operations
    bool configureBuild(); // Run cmake or equivalent
    bool build();
    bool clean();
    std::string getBuildDirectory() const;
    std::string getPreferredBuildCommand() const;

    std::string runCommand(const std::string& cmd);

private:
    std::string path_;
    std::filesystem::path buildDir_;
    mutable BuildSystem cachedBuildSystem_ = BuildSystem::None;
    mutable bool buildSystemCached_ = false;
    
    bool isExecutableFile(const std::filesystem::path& file) const;
    bool isLikelyGUIApp(const std::filesystem::path& file) const;
};

#endif // WORKSPACE_H
