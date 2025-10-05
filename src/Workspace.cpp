#include "Workspace.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>
#include <unistd.h>
#include <fstream>
#include <algorithm>
#include <regex>

Workspace::Workspace(const std::string& path) : path_(path), buildDir_(std::filesystem::path(path) / "build") {
}

std::string Workspace::getPath() const {
    return path_;
}

bool Workspace::exists() const {
    return std::filesystem::exists(path_);
}

bool Workspace::gitInit() {
    std::string cmd = "git init";
    std::string cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(path_);
    std::string result = runCommand(cmd);
    std::filesystem::current_path(cwd);
    return (result.find("Initialized empty Git repository") != std::string::npos);
}

bool Workspace::gitAdd() {
    std::string cmd = "git add .";
    std::string cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(path_);
    runCommand(cmd);
    std::filesystem::current_path(cwd);
    return true; // Assume success
}

bool Workspace::gitCommit(const std::string& message) {
    std::string cmd = "git commit -m \"" + message + "\"";
    std::string cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(path_);
    runCommand(cmd);
    std::filesystem::current_path(cwd);
    return true; // Assume success
}

bool Workspace::configureBuild() {
    std::filesystem::create_directory(buildDir_);
    std::string cmd = "cmake ..";
    std::string cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(buildDir_);
    runCommand(cmd);
    std::filesystem::current_path(cwd);
    return true; // Assume success
}

bool Workspace::build() {
    if (!std::filesystem::exists(buildDir_)) {
        configureBuild();
    }
    std::string cmd = "make";
    std::string cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(buildDir_);
    runCommand(cmd);
    std::filesystem::current_path(cwd);
    return true;
}

bool Workspace::clean() {
    std::filesystem::remove_all(buildDir_);
    return true;
}

std::string Workspace::runCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return "popen failed!";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

BuildSystem Workspace::detectBuildSystem() const {
    if (buildSystemCached_) {
        return cachedBuildSystem_;
    }
    
    std::filesystem::path basePath(path_);
    
    // Check for CMake
    if (std::filesystem::exists(basePath / "CMakeLists.txt")) {
        cachedBuildSystem_ = BuildSystem::CMake;
    }
    // Check for Makefile
    else if (std::filesystem::exists(basePath / "Makefile") || 
             std::filesystem::exists(basePath / "makefile")) {
        cachedBuildSystem_ = BuildSystem::Makefile;
    }
    // Check for Ninja
    else if (std::filesystem::exists(basePath / "build.ninja")) {
        cachedBuildSystem_ = BuildSystem::Ninja;
    }
    // Check for AutoTools
    else if (std::filesystem::exists(basePath / "configure") || 
             std::filesystem::exists(basePath / "configure.ac") ||
             std::filesystem::exists(basePath / "Makefile.am")) {
        cachedBuildSystem_ = BuildSystem::AutoTools;
    }
    // Check for build scripts
    else if (std::filesystem::exists(basePath / "build.sh") ||
             std::filesystem::exists(basePath / "build.py") ||
             std::filesystem::exists(basePath / "build.js")) {
        cachedBuildSystem_ = BuildSystem::Script;
    }
    else {
        cachedBuildSystem_ = BuildSystem::None;
    }
    
    buildSystemCached_ = true;
    return cachedBuildSystem_;
}

std::string Workspace::getBuildSystemName() const {
    switch (detectBuildSystem()) {
        case BuildSystem::CMake: return "CMake";
        case BuildSystem::Makefile: return "Makefile";
        case BuildSystem::Ninja: return "Ninja";
        case BuildSystem::AutoTools: return "AutoTools";
        case BuildSystem::Script: return "Build Script";
        case BuildSystem::None: 
        default: return "None";
    }
}

std::vector<std::string> Workspace::getBuildScripts() const {
    std::vector<std::string> scripts;
    std::filesystem::path basePath(path_);
    
    std::vector<std::string> scriptNames = {
        "build.sh", "build.py", "build.js", "build.bat",
        "compile.sh", "make.sh", "install.sh"
    };
    
    for (const auto& script : scriptNames) {
        if (std::filesystem::exists(basePath / script)) {
            scripts.push_back(script);
        }
    }
    
    return scripts;
}

std::string Workspace::getBuildDirectory() const {
    std::filesystem::path basePath(path_);
    
    // Common build directory names
    std::vector<std::string> buildDirs = {
        "build", "Build", "BUILD", "_build",
        "cmake-build", "cmake-build-debug", "cmake-build-release",
        "out", "bin", "target", "dist"
    };
    
    for (const auto& dir : buildDirs) {
        std::filesystem::path buildPath = basePath / dir;
        if (std::filesystem::exists(buildPath) && std::filesystem::is_directory(buildPath)) {
            return buildPath.string();
        }
    }
    
    // Default to build if none found
    return (basePath / "build").string();
}

std::string Workspace::getPreferredBuildCommand() const {
    switch (detectBuildSystem()) {
        case BuildSystem::CMake:
            return "cmake --build .";
        case BuildSystem::Makefile:
            return "make";
        case BuildSystem::Ninja:
            return "ninja";
        case BuildSystem::AutoTools:
            return "make";
        case BuildSystem::Script: {
            auto scripts = getBuildScripts();
            if (!scripts.empty()) {
                std::string script = scripts[0];
                if (script.size() >= 3 && script.substr(script.size() - 3) == ".sh") {
                    return "bash " + script;
                } else if (script.size() >= 3 && script.substr(script.size() - 3) == ".py") {
                    return "python " + script;
                }
                return "./" + script;
            }
            return "make";
        }
        case BuildSystem::None:
        default:
            return "make";
    }
}

bool Workspace::isExecutableFile(const std::filesystem::path& file) const {
    if (!std::filesystem::exists(file) || !std::filesystem::is_regular_file(file)) {
        return false;
    }
    
    // Check if file has execute permissions
    std::filesystem::perms perms = std::filesystem::status(file).permissions();
    bool hasExecutePerms = (perms & std::filesystem::perms::owner_exec) != std::filesystem::perms::none ||
                          (perms & std::filesystem::perms::group_exec) != std::filesystem::perms::none ||
                          (perms & std::filesystem::perms::others_exec) != std::filesystem::perms::none;
    
    if (!hasExecutePerms) return false;
    
    // Get file info
    std::string filename = file.filename().string();
    std::string extension = file.extension().string();
    std::string parent_dir = file.parent_path().filename().string();
    
    // Exclude scripts and text files
    if (extension == ".sh" || extension == ".py" || extension == ".js" || 
        extension == ".pl" || extension == ".rb" || extension == ".lua" ||
        extension == ".txt" || extension == ".md" || extension == ".json" ||
        extension == ".xml" || extension == ".yml" || extension == ".yaml") {
        return false;
    }
    
    // Exclude common system/tool executables that might be in project directories
    std::vector<std::string> excludeNames = {
        "make", "cmake", "ninja", "gcc", "g++", "clang", "clang++",
        "git", "svn", "tar", "zip", "unzip", "wget", "curl",
        "ls", "cp", "mv", "rm", "mkdir", "cat", "grep", "sed", "awk",
        "python", "python3", "node", "npm", "yarn",
        "configure", "config", "install", "setup"
    };
    
    for (const auto& exclude : excludeNames) {
        if (filename == exclude) {
            return false;
        }
    }
    
    // Exclude files in common system/cache directories
    if (parent_dir == "CMakeFiles" || parent_dir == ".git" || 
        parent_dir == "node_modules" || parent_dir == "__pycache__" ||
        parent_dir == ".cache" || parent_dir == "tmp" || parent_dir == "temp") {
        return false;
    }
    
    // For files without extensions, check if they look like built executables
    if (extension.empty()) {
        // Check file size (built executables are usually larger than a few KB)
        try {
            auto fileSize = std::filesystem::file_size(file);
            if (fileSize < 1024) { // Less than 1KB, probably not a real executable
                return false;
            }
        } catch (...) {
            return false;
        }
        
        // Simple check: read first few bytes to see if it looks like a binary
        std::ifstream f(file, std::ios::binary);
        if (f.is_open()) {
            char magic[4] = {0};
            f.read(magic, 4);
            f.close();
            
            // Check for ELF magic number (Linux executables)
            if (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F') {
                return true;
            }
            
            // Check if it starts with a shebang (script)
            if (magic[0] == '#' && magic[1] == '!') {
                return false;
            }
            
            // If it's not obviously a binary, check if it has common text patterns
            std::ifstream textCheck(file);
            std::string firstLine;
            if (std::getline(textCheck, firstLine)) {
                // If first line looks like text/script, exclude it
                if (firstLine.find("#!/") == 0 || firstLine.find("<?") == 0 ||
                    firstLine.find("//") == 0 || firstLine.find("/*") == 0 ||
                    firstLine.find("#include") != std::string::npos) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

bool Workspace::isLikelyGUIApp(const std::filesystem::path& file) const {
    // Simple heuristics - could be improved with actual binary analysis
    std::string filename = file.filename().string();
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    
    // Common GUI application indicators
    std::vector<std::string> guiIndicators = {
        "gui", "window", "qt", "gtk", "ui", "editor", "viewer", "browser"
    };
    
    for (const auto& indicator : guiIndicators) {
        if (filename.find(indicator) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

std::vector<ExecutableInfo> Workspace::findExecutables() const {
    std::vector<ExecutableInfo> executables;
    std::filesystem::path basePath(path_);
    
    // Prioritize build output directories (most likely to contain project executables)
    std::vector<std::string> searchDirs = {
        getBuildDirectory(),  // Primary build directory
        (basePath / "bin").string(),
        (basePath / "out").string(),
        (basePath / "target").string(),
        (basePath / "Release").string(),
        (basePath / "Debug").string()
    };
    
    // Only search project root if no executables found in build directories
    bool foundInBuildDirs = false;
    
    for (const auto& dirPath : searchDirs) {
        if (!std::filesystem::exists(dirPath)) continue;
        
        try {
            // Limit search depth to avoid going too deep into subdirectories
            std::filesystem::recursive_directory_iterator iter(dirPath);
            std::filesystem::recursive_directory_iterator end;
            
            for (; iter != end; ++iter) {
                // Skip deep nested directories (more than 3 levels)
                if (iter.depth() > 3) {
                    iter.disable_recursion_pending();
                    continue;
                }
                
                // Skip common directories that won't contain our executables
                std::string dirName = iter->path().parent_path().filename().string();
                if (dirName == "CMakeFiles" || dirName == ".git" || dirName == "node_modules" ||
                    dirName == "__pycache__" || dirName == ".cache" || dirName == "tmp" ||
                    dirName == "temp" || dirName == "obj" || dirName == "libs") {
                    iter.disable_recursion_pending();
                    continue;
                }
                
                if (iter->is_regular_file() && isExecutableFile(iter->path())) {
                    ExecutableInfo info;
                    info.path = iter->path().string();
                    info.name = iter->path().filename().string();
                    info.relativePath = std::filesystem::relative(iter->path(), basePath).string();
                    info.isGUI = isLikelyGUIApp(iter->path());
                    executables.push_back(info);
                    foundInBuildDirs = true;
                }
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Skip directories we can't access
            continue;
        }
    }
    
    // If no executables found in build directories, do a limited search in project root
    if (!foundInBuildDirs) {
        try {
            // Only search one level deep in project root
            for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
                if (entry.is_regular_file() && isExecutableFile(entry.path())) {
                    ExecutableInfo info;
                    info.path = entry.path().string();
                    info.name = entry.path().filename().string();
                    info.relativePath = std::filesystem::relative(entry.path(), basePath).string();
                    info.isGUI = isLikelyGUIApp(entry.path());
                    executables.push_back(info);
                }
            }
        } catch (const std::filesystem::filesystem_error&) {
            // Skip if we can't access the directory
        }
    }
    
    return executables;
}

ExecutableInfo Workspace::findMainExecutable() const {
    auto executables = findExecutables();
    
    if (executables.empty()) {
        return ExecutableInfo{};
    }
    
    // Try to find the "main" executable using heuristics
    std::string projectName = std::filesystem::path(path_).filename().string();
    std::transform(projectName.begin(), projectName.end(), projectName.begin(), ::tolower);
    
    // First, look for executables with the project name
    for (const auto& exe : executables) {
        std::string exeName = exe.name;
        std::transform(exeName.begin(), exeName.end(), exeName.begin(), ::tolower);
        if (exeName == projectName || exeName.find(projectName) != std::string::npos) {
            return exe;
        }
    }
    
    // Then look for common main executable names
    std::vector<std::string> mainNames = {"main", "app", "application", "run", "start"};
    for (const auto& mainName : mainNames) {
        for (const auto& exe : executables) {
            std::string exeName = exe.name;
            std::transform(exeName.begin(), exeName.end(), exeName.begin(), ::tolower);
            if (exeName.find(mainName) != std::string::npos) {
                return exe;
            }
        }
    }
    
    // Return the first executable found
    return executables[0];
}
