#include "WorkspaceManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>

WorkspaceManager::WorkspaceManager() {
    loadFromFile();
}

void WorkspaceManager::addWorkspace(const std::string& name, const std::string& path) {
    workspaces_[name] = std::make_unique<Workspace>(path);
    saveToFile();
}

void WorkspaceManager::removeWorkspace(const std::string& name) {
    workspaces_.erase(name);
    saveToFile();
}

Workspace* WorkspaceManager::getWorkspace(const std::string& name) {
    auto it = workspaces_.find(name);
    if (it != workspaces_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> WorkspaceManager::listWorkspaces() const {
    std::vector<std::string> names;
    for (const auto& pair : workspaces_) {
        names.push_back(pair.first + ": " + pair.second->getPath());
    }
    return names;
}

void WorkspaceManager::loadFromFile() {
    std::ifstream file(configFile_);
    std::string line;
    while (std::getline(file, line)) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string name = line.substr(0, colon);
            std::string path = line.substr(colon + 1);
            workspaces_[name] = std::make_unique<Workspace>(path);
        }
    }
}

void WorkspaceManager::saveToFile() {
    std::ofstream file(configFile_);
    for (const auto& pair : workspaces_) {
        file << pair.first << ":" << pair.second->getPath() << std::endl;
    }
}
