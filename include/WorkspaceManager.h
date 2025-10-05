#ifndef WORKSPACE_MANAGER_H
#define WORKSPACE_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Workspace.h"

class WorkspaceManager {
public:
    WorkspaceManager();
    ~WorkspaceManager() = default;

    void addWorkspace(const std::string& name, const std::string& path);
    void removeWorkspace(const std::string& name);
    Workspace* getWorkspace(const std::string& name);

    std::vector<std::string> listWorkspaces() const;
    void loadFromFile();
    void saveToFile();

private:
    std::unordered_map<std::string, std::unique_ptr<Workspace>> workspaces_;
    std::string configFile_ = "workspaces.txt"; // Simple txt file
};

#endif // WORKSPACE_MANAGER_H
