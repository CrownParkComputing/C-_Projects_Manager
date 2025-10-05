#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QDir>
#include <QStringListModel>
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QSplitter>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QUrl>
#include <QDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QRegularExpression>
#include <QLineEdit>
#include <QProgressBar>
#include <QCloseEvent>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentWorkspace_(nullptr), buildProcess_(nullptr), buildOutput_(nullptr), buildProgress_(nullptr), isGithubAuthenticated_(false) {
    setupUI();
}

MainWindow::~MainWindow() {
    // Save workspaces before exiting
    wm_.saveToFile();
    
    // Clean up build process if running
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        buildProcess_->kill();
        buildProcess_->waitForFinished(3000);
    }
}

void MainWindow::setupUI() {
    setWindowTitle("C++ Workspace Manager");
    resize(1200, 800);
    
    // Apply modern dark theme with colors
    setStyleSheet(R"(
        QMainWindow {
            background-color: #2b2b2b;
            color: #ffffff;
        }
        QListWidget {
            background-color: #3c3c3c;
            border: 2px solid #4a90e2;
            border-radius: 8px;
            color: #ffffff;
            selection-background-color: #4a90e2;
            font-size: 12px;
            padding: 5px;
        }
        QListWidget::item {
            border-bottom: 1px solid #555;
            padding: 8px;
            margin: 2px;
        }
        QListWidget::item:selected {
            background-color: #4a90e2;
            color: white;
            border-radius: 4px;
        }
        QListWidget::item:hover {
            background-color: #5a5a5a;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: bold;
            font-size: 11px;
            min-height: 20px;
        }
        QPushButton:hover {
            background-color: #357abd;
        }
        QPushButton:pressed {
            background-color: #2968a3;
        }
        QPushButton:disabled {
            background-color: #666666;
            color: #999999;
        }
        QPushButton#dangerButton {
            background-color: #e74c3c;
        }
        QPushButton#dangerButton:hover {
            background-color: #c0392b;
        }
        QPushButton#successButton {
            background-color: #27ae60;
        }
        QPushButton#successButton:hover {
            background-color: #229954;
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #555;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 10px 0 10px;
            color: #4a90e2;
        }
        QComboBox {
            background-color: #3c3c3c;
            border: 2px solid #555;
            border-radius: 4px;
            padding: 5px;
            color: #ffffff;
        }
        QComboBox:hover {
            border-color: #4a90e2;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #ffffff;
        }
        QLabel {
            color: #ffffff;
        }
        QTextEdit {
            background-color: #1e1e1e;
            border: 2px solid #555;
            border-radius: 6px;
            color: #ffffff;
            font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 10px;
        }
        QProgressBar {
            border: 2px solid #555;
            border-radius: 6px;
            text-align: center;
            background-color: #3c3c3c;
            color: #ffffff;
        }
        QProgressBar::chunk {
            background-color: #4a90e2;
            border-radius: 4px;
        }
        QSplitter::handle {
            background-color: #555;
        }
        QSplitter::handle:horizontal {
            width: 4px;
        }
        QSplitter::handle:vertical {
            height: 4px;
        }
        QMenuBar {
            background-color: #2b2b2b;
            color: #ffffff;
            border-bottom: 1px solid #555;
        }
        QMenuBar::item {
            background-color: transparent;
            padding: 8px 16px;
        }
        QMenuBar::item:selected {
            background-color: #4a90e2;
            border-radius: 4px;
        }
        QMenu {
            background-color: #3c3c3c;
            border: 1px solid #555;
            color: #ffffff;
        }
        QMenu::item {
            padding: 8px 20px;
        }
        QMenu::item:selected {
            background-color: #4a90e2;
        }
    )");

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Left side: workspace list and add button
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    addLocalButton_ = new QPushButton("Add Local Folder", leftWidget);
    addLocalButton_->setObjectName("successButton");
    cloneGithubButton_ = new QPushButton("Clone from GitHub", leftWidget);
    cloneGithubButton_->setObjectName("successButton");
    removeButton_ = new QPushButton("Remove Workspace", leftWidget);
    removeButton_->setObjectName("dangerButton");
    leftLayout->addWidget(addLocalButton_);
    leftLayout->addWidget(cloneGithubButton_);
    leftLayout->addWidget(removeButton_);

    workspaceList_ = new QListWidget(leftWidget);
    leftLayout->addWidget(workspaceList_);

    mainLayout->addWidget(leftWidget);

    // Right side: splitter for info display and action panels
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, centralWidget);
    
    // Info display area
    infoDisplay_ = new QTextEdit();
    infoDisplay_->setReadOnly(true);
    rightSplitter->addWidget(infoDisplay_);

    // Build output area
    buildOutput_ = new QTextEdit();
    buildOutput_->setReadOnly(true);
    rightSplitter->addWidget(buildOutput_);

    // Action panels container
    QWidget *actionsWidget = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);

    // Build management group
    buildGroup_ = new QGroupBox("Build Management");
    QVBoxLayout *buildLayout = new QVBoxLayout(buildGroup_);

    buildProgress_ = new QProgressBar(buildGroup_);
    buildProgress_->setRange(0, 100);
    buildProgress_->setVisible(false);
    buildLayout->addWidget(buildProgress_);

    // Build system selection
    buildSystemLabel_ = new QLabel("Build System:");
    buildLayout->addWidget(buildSystemLabel_);
    
    buildSystemCombo_ = new QComboBox();
    buildSystemCombo_->addItem("Make");
    buildSystemCombo_->addItem("Ninja");
    buildSystemCombo_->setCurrentText("Make");
    buildLayout->addWidget(buildSystemCombo_);

    buildButton_ = new QPushButton("Build");
    buildButton_->setObjectName("successButton");
    cleanButton_ = new QPushButton("Clean");
    cleanButton_->setObjectName("dangerButton");
    runButton_ = new QPushButton("Run");
    runButton_->setObjectName("successButton");
    editMakefileButton_ = new QPushButton("Edit Makefile");

    buildLayout->addWidget(buildButton_);
    buildLayout->addWidget(cleanButton_);
    buildLayout->addWidget(runButton_);
    buildLayout->addWidget(editMakefileButton_);

    actionsLayout->addWidget(buildGroup_);

    // Git versioning group
    gitGroup_ = new QGroupBox("Git Versioning");
    QVBoxLayout *gitLayout = new QVBoxLayout(gitGroup_);

    versionLabel_ = new QLabel("Current Version: N/A");
    gitLayout->addWidget(versionLabel_);

    QHBoxLayout *versionButtonsLayout = new QHBoxLayout();
    majorButton_ = new QPushButton("Major");
    minorButton_ = new QPushButton("Minor");
    patchButton_ = new QPushButton("Patch");

    versionButtonsLayout->addWidget(majorButton_);
    versionButtonsLayout->addWidget(minorButton_);
    versionButtonsLayout->addWidget(patchButton_);
    gitLayout->addLayout(versionButtonsLayout);

    branchCombo_ = new QComboBox();
    branchCombo_->addItems({"main", "master", "develop"});
    gitLayout->addWidget(new QLabel("Branch:"));
    gitLayout->addWidget(branchCombo_);

    pushButton_ = new QPushButton("Push");
    commitPushButton_ = new QPushButton("Commit & Push");
    renameRepoButton_ = new QPushButton("Rename Repo");
    createReleaseButton_ = new QPushButton("Create Release");
    openGithubButton_ = new QPushButton("Open on GitHub");
    viewIssuesButton_ = new QPushButton("View Issues");
    githubAuthButton_ = new QPushButton("GitHub Auth");
    generateTokenButton_ = new QPushButton("Generate Token");
    generateTokenButton_->setToolTip("Click to open GitHub Personal Access Token generation page");
    browseReposButton_ = new QPushButton("Browse My Repos");
    browseReposButton_->setObjectName("successButton");
    createRepoButton_ = new QPushButton("Create Repo");
    createRepoButton_->setObjectName("successButton");

    gitLayout->addWidget(pushButton_);
    gitLayout->addWidget(commitPushButton_);
    gitLayout->addWidget(renameRepoButton_);
    gitLayout->addWidget(createReleaseButton_);
    gitLayout->addWidget(openGithubButton_);
    gitLayout->addWidget(viewIssuesButton_);
    gitLayout->addWidget(githubAuthButton_);
    gitLayout->addWidget(generateTokenButton_);
    gitLayout->addWidget(browseReposButton_);
    gitLayout->addWidget(createRepoButton_);

    actionsLayout->addWidget(gitGroup_);

    // Script management group
    scriptGroup_ = new QGroupBox("Script Management");
    QVBoxLayout *scriptLayout = new QVBoxLayout(scriptGroup_);

    scriptLayout->addWidget(new QLabel("Available Scripts:"));
    scriptCombo_ = new QComboBox();
    scriptLayout->addWidget(scriptCombo_);

    runScriptButton_ = new QPushButton("Run Script");
    installSystemWideButton_ = new QPushButton("Install System-Wide");

    scriptLayout->addWidget(runScriptButton_);
    scriptLayout->addWidget(installSystemWideButton_);

    actionsLayout->addWidget(scriptGroup_);

    refreshButton_ = new QPushButton("Refresh", actionsWidget);
    actionsLayout->addWidget(refreshButton_);

    rightSplitter->addWidget(actionsWidget);
    rightSplitter->setStretchFactor(0, 2);
    rightSplitter->setStretchFactor(1, 1);
    rightSplitter->setStretchFactor(2, 1);
    
    mainLayout->addWidget(rightSplitter);
    mainLayout->setStretchFactor(leftWidget, 1);
    mainLayout->setStretchFactor(rightSplitter, 3);

    // Connect signals
    connect(addLocalButton_, &QPushButton::clicked, this, &MainWindow::addLocalWorkspace);
    connect(cloneGithubButton_, &QPushButton::clicked, this, &MainWindow::cloneGithubRepo);
    connect(removeButton_, &QPushButton::clicked, this, &MainWindow::removeWorkspace);
    connect(workspaceList_, &QListWidget::itemClicked, this, &MainWindow::selectWorkspace);
    connect(refreshButton_, &QPushButton::clicked, this, &MainWindow::refreshWorkspace);
    
    // Build management connections
    connect(buildButton_, &QPushButton::clicked, this, &MainWindow::buildWorkspace);
    connect(cleanButton_, &QPushButton::clicked, this, &MainWindow::cleanWorkspace);
    connect(runButton_, &QPushButton::clicked, this, &MainWindow::runWorkspace);
    connect(editMakefileButton_, &QPushButton::clicked, this, &MainWindow::editMakefile);
    connect(buildSystemCombo_, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::setBuildSystem);
    
    // Git versioning connections
    connect(majorButton_, &QPushButton::clicked, this, &MainWindow::gitMajorVersion);
    connect(minorButton_, &QPushButton::clicked, this, &MainWindow::gitMinorVersion);
    connect(patchButton_, &QPushButton::clicked, this, &MainWindow::gitPatchVersion);
    connect(pushButton_, &QPushButton::clicked, this, &MainWindow::gitPush);
    connect(commitPushButton_, &QPushButton::clicked, this, &MainWindow::gitCommitAndPush);
    connect(renameRepoButton_, &QPushButton::clicked, this, &MainWindow::renameRepository);
    connect(createReleaseButton_, &QPushButton::clicked, this, &MainWindow::createGithubRelease);
    connect(openGithubButton_, &QPushButton::clicked, this, &MainWindow::openGithubRepo);
    connect(viewIssuesButton_, &QPushButton::clicked, this, &MainWindow::viewGithubIssues);
    connect(githubAuthButton_, &QPushButton::clicked, this, &MainWindow::authenticateGithub);
    connect(generateTokenButton_, &QPushButton::clicked, this, &MainWindow::openGithubTokenPage);
    connect(browseReposButton_, &QPushButton::clicked, this, &MainWindow::browseGithubRepos);
    connect(createRepoButton_, &QPushButton::clicked, this, &MainWindow::createGithubRepo);

    // Script management connections
    connect(runScriptButton_, &QPushButton::clicked, this, &MainWindow::runScript);
    connect(installSystemWideButton_, &QPushButton::clicked, this, &MainWindow::installSystemWide);

    // Initially disable action buttons
    updateActionButtons();

    // Initially disable GitHub authenticated features
    browseReposButton_->setEnabled(false);
    createRepoButton_->setEnabled(false);
    removeButton_->setEnabled(false);

    // Discover and populate available scripts
    discoverScripts();

    // Load existing workspaces
    auto workspaces = wm_.listWorkspaces();
    for (const auto& ws : workspaces) {
        workspaceList_->addItem(QString::fromStdString(ws));
    }
}

void MainWindow::addWorkspace() {
    // This function is kept for compatibility but redirects to addLocalWorkspace
    addLocalWorkspace();
}

void MainWindow::addLocalWorkspace() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Workspace Directory", QDir::homePath());
    if (!path.isEmpty()) {
        // Automatically generate workspace name from folder name
        QDir dir(path);
        QString name = dir.dirName();
        
        // If the directory name is empty (root), use the last part of the path
        if (name.isEmpty()) {
            QStringList pathParts = path.split("/", Qt::SkipEmptyParts);
            if (!pathParts.isEmpty()) {
                name = pathParts.last();
            } else {
                name = "Root";
            }
        }
        
        wm_.addWorkspace(name.toStdString(), path.toStdString());
        workspaceList_->addItem(name + ": " + path);
        
        // Select the newly added workspace
        workspaceList_->setCurrentRow(workspaceList_->count() - 1);
        currentWorkspaceName_ = name;
        currentWorkspace_ = wm_.getWorkspace(name.toStdString());
    }
}

void MainWindow::cloneGithubRepo() {
    QString repoUrl = QInputDialog::getText(this, "Clone GitHub Repository", 
                                          "GitHub Repository URL:\n(https://github.com/user/repo or git@github.com:user/repo.git)");
    
    if (repoUrl.isEmpty()) {
        return;
    }
    
    // Extract repository name from URL
    QString repoName;
    QRegularExpression urlPattern(R"((?:https://github\.com/|git@github\.com:)([^/]+)/([^/\.]+)(?:\.git)?)");
    QRegularExpressionMatch match = urlPattern.match(repoUrl);
    
    if (match.hasMatch()) {
        repoName = match.captured(2); // Repository name
    } else {
        QMessageBox::warning(this, "Invalid URL", "Please enter a valid GitHub repository URL.");
        return;
    }
    
    // Choose destination directory
    QString baseDir = QFileDialog::getExistingDirectory(this, "Select Directory to Clone Into", QDir::homePath());
    if (baseDir.isEmpty()) {
        return;
    }
    
    QString clonePath = baseDir + "/" + repoName;
    
    // Check if directory already exists
    if (QDir(clonePath).exists()) {
        int ret = QMessageBox::question(this, "Directory Exists", 
                                      QString("Directory '%1' already exists. Continue anyway?").arg(clonePath),
                                      QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    // Show progress and start clone process
    buildOutput_->clear();
    buildOutput_->append(QString("=== Cloning repository: %1 ===").arg(repoUrl));
    buildOutput_->append(QString("Destination: %1").arg(clonePath));
    buildOutput_->append("");
    
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "Process Running", "Another process is already running. Please wait for it to finish.");
        return;
    }
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, repoName, clonePath](int exitCode, QProcess::ExitStatus exitStatus) {
                onCloneFinished(exitCode, exitStatus, repoName, clonePath);
            });
    connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onScriptOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onScriptOutput);
    
    buildProgress_->setVisible(true);
    buildProgress_->setRange(0, 0); // Indeterminate progress
    
    // Start git clone
    buildProcess_->setWorkingDirectory(baseDir);
    buildProcess_->start("git", QStringList() << "clone" << repoUrl << repoName);
    
    if (!buildProcess_->waitForStarted()) {
        QMessageBox::critical(this, "Clone Failed", "Failed to start git clone process.");
        buildProgress_->setVisible(false);
        buildProcess_->deleteLater();
        buildProcess_ = nullptr;
    }
}

void MainWindow::removeWorkspace() {
    QListWidgetItem* currentItem = workspaceList_->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "No Workspace Selected", "Please select a workspace to remove.");
        return;
    }
    
    QString text = currentItem->text();
    size_t colon = text.indexOf(':');
    QString name = text.left(colon);
    QString path = text.mid(colon + 2); // +2 to skip ": "
    
    int ret = QMessageBox::question(this, "Remove Workspace", 
                                   QString("Are you sure you want to remove workspace '%1'?\n\n"
                                          "Path: %2\n\n"
                                          "Note: This will only remove it from the workspace list. "
                                          "The actual files will not be deleted.").arg(name, path),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Remove from workspace manager
        wm_.removeWorkspace(name.toStdString());
        
        // Remove from UI list
        int row = workspaceList_->row(currentItem);
        delete workspaceList_->takeItem(row);
        
        // Clear current workspace if it was the removed one
        if (currentWorkspaceName_ == name) {
            currentWorkspace_ = nullptr;
            currentWorkspaceName_.clear();
            infoDisplay_->clear();
            infoDisplay_->setText("No workspace selected");
        }
        
        // Update button states
        updateActionButtons();
        
        QMessageBox::information(this, "Workspace Removed", 
                               QString("Workspace '%1' has been removed from the list.").arg(name));
    }
}

void MainWindow::onBuildFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        QMessageBox::information(this, "Build Complete", "Build completed successfully!");
    } else {
        QMessageBox::warning(this, "Build Failed", "Build failed. Check the project configuration.");
    }

    buildProgress_->setVisible(false);
    buildProcess_->deleteLater();
    buildProcess_ = nullptr;
}

void MainWindow::onBuildOutput() {
    QString output = buildProcess_->readAllStandardOutput();
    buildOutput_->append(output);

    // Parse progress
    if (output.contains('[', Qt::CaseInsensitive) && output.contains('%')) {
        int start = output.indexOf('[');
        if (start != -1) {
            int pct = output.mid(start + 1).indexOf('%');
            if (pct != -1) {
                QString progressStr = output.mid(start + 1, pct - start);
                bool ok;
                int progress = progressStr.toInt(&ok);
                if (ok && progress >= 0 && progress <= 100) {
                    buildProgress_->setValue(progress);
                }
            }
        }
    }

    QString error = buildProcess_->readAllStandardError();
    if (!error.isEmpty()) {
        buildOutput_->append(error);
    }
}

void MainWindow::onCmakeFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        buildOutput_->append("CMake configuration completed successfully!\n");
        buildOutput_->append("Starting make build...\n");
        startMakeBuild();
    } else {
        buildOutput_->append("CMake configuration failed!\n");
        buildProgress_->setVisible(false);
        QMessageBox::warning(this, "CMake Failed", "CMake configuration failed. Check the build output for details.");
        
        if (buildProcess_) {
            buildProcess_->deleteLater();
            buildProcess_ = nullptr;
        }
    }
}

void MainWindow::startMakeBuild() {
    if (!currentWorkspace_) return;
    
    QString buildDir = QString::fromStdString(currentWorkspace_->getPath()) + "/build";
    
    if (buildProcess_) {
        buildProcess_->deleteLater();
    }
    
    buildProcess_ = new QProcess(this);
    buildProcess_->setWorkingDirectory(buildDir);
    
    connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onBuildOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onBuildOutput);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onBuildFinished);
    
    buildOutput_->append("Running make...\n");
    buildProcess_->start("make", QStringList());
}

void MainWindow::selectWorkspace(QListWidgetItem* item) {
    QString text = item->text();
    size_t colon = text.indexOf(':');
    QString name = text.left(colon);
    currentWorkspaceName_ = name;
    currentWorkspace_ = wm_.getWorkspace(name.toStdString());
    if (currentWorkspace_) {
        displayWorkspaceInfo(currentWorkspace_);
        updateActionButtons();
    }
    removeButton_->setEnabled(true);
}

void MainWindow::refreshWorkspace() {
    if (currentWorkspace_) {
        displayWorkspaceInfo(currentWorkspace_);
    }
}

void MainWindow::displayWorkspaceInfo(Workspace* ws) {
    infoDisplay_->clear();
    QString info = "Path: " + QString::fromStdString(ws->getPath()) + "\n\n";

    // Build System Information
    info += "=== Build System ===\n";
    info += "Type: " + QString::fromStdString(ws->getBuildSystemName()) + "\n";
    info += "Build Directory: " + QString::fromStdString(ws->getBuildDirectory()) + "\n";
    info += "Build Command: " + QString::fromStdString(ws->getPreferredBuildCommand()) + "\n";
    
    auto buildScripts = ws->getBuildScripts();
    if (!buildScripts.empty()) {
        info += "Build Scripts: ";
        for (size_t i = 0; i < buildScripts.size(); ++i) {
            info += QString::fromStdString(buildScripts[i]);
            if (i < buildScripts.size() - 1) info += ", ";
        }
        info += "\n";
    }
    info += "\n";

    // Executable Information
    info += "=== Executables ===\n";
    auto executables = ws->findExecutables();
    if (executables.empty()) {
        info += "No executables found\n";
    } else {
        info += QString("Found %1 executable(s):\n").arg(executables.size());
        for (const auto& exe : executables) {
            info += QString("- %1 (%2)%3\n")
                   .arg(QString::fromStdString(exe.name))
                   .arg(QString::fromStdString(exe.relativePath))
                   .arg(exe.isGUI ? " [GUI]" : "");
        }
        
        auto mainExe = ws->findMainExecutable();
        if (!mainExe.name.empty()) {
            info += QString("Main: %1\n").arg(QString::fromStdString(mainExe.name));
        }
    }
    info += "\n";

    // Git Information
    info += "=== Git Information ===\n";
    std::string cmd = "git remote -v";
    std::string cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(ws->getPath());
    std::string result = ws->runCommand(cmd);
    std::filesystem::current_path(cwd);
    
    if (!result.empty() && result.find("fatal") == std::string::npos) {
        QString remoteUrl = QString::fromStdString(result).split('\n')[0];
        if (!remoteUrl.trimmed().isEmpty()) {
            info += "Remote URL: " + remoteUrl + "\n";
        }
    } else {
        info += "Remote URL: Not a git repository\n";
    }

    // Git tags
    cmd = "git describe --tags --abbrev=0";
    cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(ws->getPath());
    result = ws->runCommand(cmd);
    std::filesystem::current_path(cwd);
    
    if (!result.empty() && result.find("fatal") == std::string::npos) {
        info += "Latest Tag: " + QString::fromStdString(result).trimmed() + "\n";
    } else {
        info += "Latest Tag: None\n";
    }

    // Git status
    cmd = "git status --porcelain";
    cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(ws->getPath());
    result = ws->runCommand(cmd);
    std::filesystem::current_path(cwd);
    QStringList fileStatus = QString::fromStdString(result).split('\n', Qt::SkipEmptyParts);
    
    if (fileStatus.isEmpty()) {
        info += "Status: Clean working tree\n";
    } else {
        info += QString("Status: %1 changed file(s)\n").arg(fileStatus.size());
    }
    info += "\n";

    // Project Structure
    info += "=== Project Structure ===\n";
    
    // Actions scripts (GitHub actions)
    std::filesystem::path actionsPath = std::filesystem::path(ws->getPath()) / ".github" / "workflows";
    if (std::filesystem::exists(actionsPath)) {
        info += "GitHub Actions: Available\n";
    } else {
        info += "GitHub Actions: None\n";
    }

    // Scripts directory
    std::filesystem::path scriptsPath = std::filesystem::path(ws->getPath()) / "scripts";
    if (std::filesystem::exists(scriptsPath)) {
        info += "Scripts Directory: Available\n";
    } else {
        info += "Scripts Directory: None\n";
    }

    // Documentation
    std::filesystem::path docsPath = std::filesystem::path(ws->getPath()) / "docs";
    std::filesystem::path readmePath = std::filesystem::path(ws->getPath()) / "README.md";
    if (std::filesystem::exists(docsPath)) {
        info += "Documentation: docs/ directory\n";
    } else if (std::filesystem::exists(readmePath)) {
        info += "Documentation: README.md\n";
    } else {
        info += "Documentation: None\n";
    }

    infoDisplay_->setText(info);
    
    // Update version label
    QString version = getCurrentVersion(ws);
    versionLabel_->setText("Current Version: " + version);
}

void MainWindow::updateActionButtons() {
    bool hasWorkspace = currentWorkspace_ != nullptr;
    bool hasWorkspaceInList = workspaceList_->currentItem() != nullptr;

    buildButton_->setEnabled(hasWorkspace);
    cleanButton_->setEnabled(hasWorkspace);
    runButton_->setEnabled(hasWorkspace);
    removeButton_->setEnabled(hasWorkspaceInList);

    majorButton_->setEnabled(hasWorkspace);
    minorButton_->setEnabled(hasWorkspace);
    patchButton_->setEnabled(hasWorkspace);
    pushButton_->setEnabled(hasWorkspace);
    commitPushButton_->setEnabled(hasWorkspace);
    branchCombo_->setEnabled(hasWorkspace);

    if (buildOutput_) {
        buildOutput_->setEnabled(hasWorkspace);
    }

    if (refreshButton_) {
        refreshButton_->setEnabled(hasWorkspace);
    }
}

QString MainWindow::getCurrentVersion(Workspace* ws) {
    if (!ws) return "N/A";
    
    std::string cmd = "git describe --tags --abbrev=0";
    std::string cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(ws->getPath());
    std::string result = ws->runCommand(cmd);
    std::filesystem::current_path(cwd);
    
    if (result.empty() || result.find("fatal") != std::string::npos) {
        return "0.0.0";
    }
    
    QString version = QString::fromStdString(result).trimmed();
    if (version.startsWith("v")) {
        version = version.mid(1);
    }
    return version;
}

QString MainWindow::incrementVersion(const QString& version, int type) {
    QStringList parts = version.split('.');
    if (parts.size() != 3) {
        return "1.0.0";
    }
    
    int major = parts[0].toInt();
    int minor = parts[1].toInt();
    int patch = parts[2].toInt();
    
    switch(type) {
        case 2: // Major
            major++;
            minor = 0;
            patch = 0;
            break;
        case 1: // Minor
            minor++;
            patch = 0;
            break;
        case 0: // Patch
        default:
            patch++;
            break;
    }
    
    return QString("%1.%2.%3").arg(major).arg(minor).arg(patch);
}

// Build management actions
void MainWindow::buildWorkspace() {
    if (!currentWorkspace_) return;

    if (buildProcess_ && buildProcess_->state() == QProcess::Running) {
        QMessageBox::warning(this, "Build in Progress", "Another build is already running.");
        return;
    }

    buildOutput_->clear();
    
    // Get build system information
    QString buildSystemName = QString::fromStdString(currentWorkspace_->getBuildSystemName());
    QString buildCommand = QString::fromStdString(currentWorkspace_->getPreferredBuildCommand());
    QString buildDir = QString::fromStdString(currentWorkspace_->getBuildDirectory());
    QString workspacePath = QString::fromStdString(currentWorkspace_->getPath());
    
    buildOutput_->append(QString("Starting build for workspace: %1\n").arg(workspacePath));
    buildOutput_->append(QString("Build System: %1\n").arg(buildSystemName));
    buildOutput_->append(QString("Build Directory: %1\n").arg(buildDir));
    buildOutput_->append(QString("Build Command: %1\n\n").arg(buildCommand));

    buildProgress_->setValue(0);
    buildProgress_->setVisible(true);

    // Create build directory if it doesn't exist
    QDir dir;
    if (!dir.exists(buildDir)) {
        buildOutput_->append("Creating build directory...\n");
        if (!dir.mkpath(buildDir)) {
            buildOutput_->append("ERROR: Failed to create build directory!\n");
            buildProgress_->setVisible(false);
            QMessageBox::warning(this, "Build Failed", "Failed to create build directory.");
            return;
        }
    }

    // Set up the build process
    buildProcess_ = new QProcess(this);
    
    // Parse the build command
    QStringList commandParts = buildCommand.split(' ', Qt::SkipEmptyParts);
    if (commandParts.isEmpty()) {
        buildOutput_->append("ERROR: Invalid build command!\n");
        buildProgress_->setVisible(false);
        return;
    }
    
    QString program = commandParts[0];
    QStringList arguments = commandParts.mid(1);
    
    // Handle special cases for different build systems
    BuildSystem buildSystem = currentWorkspace_->detectBuildSystem();
    QString selectedBuildSystem = buildSystemCombo_->currentText();
    
    if (buildSystem == BuildSystem::CMake) {
        // For CMake projects, check if we need to configure first
        if (!QFile::exists(buildDir + "/CMakeCache.txt")) {
            buildOutput_->append("Running CMake configuration...\n");
            buildProcess_->setWorkingDirectory(buildDir);
            
            connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onBuildOutput);
            connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onBuildOutput);
            connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    this, &MainWindow::onCmakeFinished);
            
            // Configure CMake with selected generator
            QStringList cmakeArgs;
            cmakeArgs << "..";
            if (selectedBuildSystem == "Ninja") {
                cmakeArgs << "-G" << "Ninja";
            } else {
                cmakeArgs << "-G" << "Unix Makefiles";
            }
            
            buildProcess_->start("cmake", cmakeArgs);
            return;
        } else {
            // CMake already configured, use cmake --build with appropriate backend
            buildProcess_->setWorkingDirectory(buildDir);
            program = "cmake";
            arguments = QStringList() << "--build" << ".";
            
            // Add parallel jobs for the selected build system
            if (selectedBuildSystem == "Ninja") {
                arguments << "--parallel";
            } else {
                arguments << "--parallel" << QString::number(QThread::idealThreadCount());
            }
        }
    } else if (buildSystem == BuildSystem::Script) {
        // For build scripts, run from the project root
        buildProcess_->setWorkingDirectory(workspacePath);
    } else {
        // For Make, Ninja, etc., run from build directory
        buildProcess_->setWorkingDirectory(buildDir);
    }

    connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onBuildOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onBuildOutput);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onBuildFinished);
    
    buildOutput_->append(QString("Executing: %1 %2\n").arg(program, arguments.join(" ")));
    buildProcess_->start(program, arguments);
}

void MainWindow::cleanWorkspace() {
    if (!currentWorkspace_) return;
    
    int ret = QMessageBox::question(this, "Clean Workspace", 
                                   "Are you sure you want to clean the build directory?",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        bool success = currentWorkspace_->clean();
        if (success) {
            QMessageBox::information(this, "Clean Complete", "Build directory cleaned successfully!");
        } else {
            QMessageBox::warning(this, "Clean Failed", "Failed to clean build directory.");
        }
    }
}

void MainWindow::runWorkspace() {
    if (!currentWorkspace_) return;

    // Find executables in the workspace
    auto executables = currentWorkspace_->findExecutables();
    
    if (executables.empty()) {
        QMessageBox::warning(this, "No Executable Found", 
                           "No executable files found in the workspace.\n"
                           "Make sure the project has been built successfully.");
        return;
    }

    ExecutableInfo execToRun;
    
    if (executables.size() == 1) {
        // Only one executable, use it
        execToRun = executables[0];
    } else {
        // Multiple executables, try to find the main one
        execToRun = currentWorkspace_->findMainExecutable();
        
        if (execToRun.name.empty()) {
            // Ask user to choose
            QStringList execNames;
            for (const auto& exe : executables) {
                execNames << QString("%1 (%2)%3")
                            .arg(QString::fromStdString(exe.name))
                            .arg(QString::fromStdString(exe.relativePath))
                            .arg(exe.isGUI ? " [GUI]" : "");
            }
            
            bool ok;
            QString selected = QInputDialog::getItem(this, "Select Executable", 
                                                   "Multiple executables found. Select one to run:",
                                                   execNames, 0, false, &ok);
            
            if (!ok) return;
            
            int index = execNames.indexOf(selected);
            if (index >= 0 && index < executables.size()) {
                execToRun = executables[index];
            } else {
                return;
            }
        }
    }

    QString exePath = QString::fromStdString(execToRun.path);
    
    if (!QFile::exists(exePath)) {
        QMessageBox::warning(this, "Executable Not Found", 
                           QString("The executable was not found at: %1\n"
                                  "Make sure the project has been built successfully.").arg(exePath));
        return;
    }

    // Prepare environment
    QStringList environment = QProcess::systemEnvironment();
    QString workingDir = QString::fromStdString(currentWorkspace_->getPath());
    
    // Add build directory to PATH if needed
    QString buildDir = QString::fromStdString(currentWorkspace_->getBuildDirectory());
    QStringList pathEntries = environment.filter("PATH=");
    QString pathVar = pathEntries.isEmpty() ? 
                     QString("PATH=%1").arg(buildDir) :
                     pathEntries.first() + ":" + buildDir;
    
    // Remove existing PATH entries
    for (const QString& entry : pathEntries) {
        environment.removeAll(entry);
    }
    environment << pathVar;

    buildOutput_->append(QString("\n=== Running Application ===\n"));
    buildOutput_->append(QString("Executable: %1\n").arg(QString::fromStdString(execToRun.name)));
    buildOutput_->append(QString("Path: %1\n").arg(exePath));
    buildOutput_->append(QString("Working Directory: %1\n").arg(workingDir));
    buildOutput_->append(QString("Type: %1\n\n").arg(execToRun.isGUI ? "GUI Application" : "Console Application"));

    // Start the process
    QProcess* runProcess = new QProcess(this);
    runProcess->setWorkingDirectory(workingDir);
    runProcess->setEnvironment(environment);
    
    if (execToRun.isGUI) {
        // For GUI applications, start detached so they don't block
        if (QProcess::startDetached(exePath, QStringList(), workingDir, nullptr)) {
            buildOutput_->append("Application started successfully!\n");
            QMessageBox::information(this, "Application Started", 
                                   QString("Started: %1").arg(QString::fromStdString(execToRun.name)));
        } else {
            buildOutput_->append("Failed to start application!\n");
            QMessageBox::warning(this, "Launch Failed", "Failed to start the application.");
        }
        runProcess->deleteLater();
    } else {
        // For console applications, run and show output
        connect(runProcess, &QProcess::readyReadStandardOutput, [this, runProcess]() {
            buildOutput_->append(runProcess->readAllStandardOutput());
        });
        
        connect(runProcess, &QProcess::readyReadStandardError, [this, runProcess]() {
            buildOutput_->append(runProcess->readAllStandardError());
        });
        
        connect(runProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [this, runProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                    buildOutput_->append(QString("\nApplication finished with exit code %1\n").arg(exitCode));
                    runProcess->deleteLater();
                });
        
        buildOutput_->append("Starting console application...\n");
        runProcess->start(exePath, QStringList());
        
        if (!runProcess->waitForStarted(3000)) {
            buildOutput_->append("Failed to start application!\n");
            QMessageBox::warning(this, "Launch Failed", "Failed to start the application.");
            runProcess->deleteLater();
        }
    }
}

// Git versioning actions
void MainWindow::gitMajorVersion() {
    if (!currentWorkspace_) return;
    
    QString currentVer = getCurrentVersion(currentWorkspace_);
    QString newVer = incrementVersion(currentVer, 2);
    
    int ret = QMessageBox::question(this, "Create Major Version", 
                                   QString("Create new major version tag?\n%1 -> %2").arg(currentVer, newVer),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        createVersionTag(newVer);
    }
}

void MainWindow::gitMinorVersion() {
    if (!currentWorkspace_) return;
    
    QString currentVer = getCurrentVersion(currentWorkspace_);
    QString newVer = incrementVersion(currentVer, 1);
    
    int ret = QMessageBox::question(this, "Create Minor Version", 
                                   QString("Create new minor version tag?\n%1 -> %2").arg(currentVer, newVer),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        createVersionTag(newVer);
    }
}

void MainWindow::gitPatchVersion() {
    if (!currentWorkspace_) return;
    
    QString currentVer = getCurrentVersion(currentWorkspace_);
    QString newVer = incrementVersion(currentVer, 0);
    
    int ret = QMessageBox::question(this, "Create Patch Version", 
                                   QString("Create new patch version tag?\n%1 -> %2").arg(currentVer, newVer),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        createVersionTag(newVer);
    }
}

void MainWindow::createVersionTag(const QString& version) {
    if (!currentWorkspace_) return;
    
    std::string cwd = std::filesystem::current_path().string();
    std::filesystem::current_path(currentWorkspace_->getPath());
    
    // Create annotated tag
    std::string cmd = "git tag -a v" + version.toStdString() + " -m \"Version " + version.toStdString() + "\"";
    std::string result = currentWorkspace_->runCommand(cmd);
    
    std::filesystem::current_path(cwd);
    
    if (result.find("fatal") == std::string::npos) {
        QMessageBox::information(this, "Version Tagged", "Version " + version + " tagged successfully!");
        versionLabel_->setText("Current Version: " + version);
    } else {
        QMessageBox::warning(this, "Tag Failed", "Failed to create version tag.\n" + QString::fromStdString(result));
    }
}

void MainWindow::gitPush() {
    if (!currentWorkspace_) return;
    
    QString branch = branchCombo_->currentText();
    int ret = QMessageBox::question(this, "Git Push", 
                                   "Push to branch: " + branch + " with tags?",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        std::string cwd = std::filesystem::current_path().string();
        std::filesystem::current_path(currentWorkspace_->getPath());
        
        // Push branch and tags
        std::string cmd = "git push origin " + branch.toStdString() + " --tags";
        std::string result = currentWorkspace_->runCommand(cmd);
        
        std::filesystem::current_path(cwd);
        
        if (result.find("fatal") == std::string::npos && result.find("error") == std::string::npos) {
            QMessageBox::information(this, "Push Complete", "Successfully pushed to " + branch + " with tags!");
            refreshWorkspace();
        } else {
            QMessageBox::warning(this, "Push Failed", "Failed to push.\n" + QString::fromStdString(result));
        }
    }
}

void MainWindow::gitCommitAndPush() {
    if (!currentWorkspace_) return;
    
    bool ok;
    QString commitMsg = QInputDialog::getText(this, "Commit Message", 
                                            "Enter commit message:", QLineEdit::Normal, "", &ok);
    
    if (ok && !commitMsg.isEmpty()) {
        std::string cwd = std::filesystem::current_path().string();
        std::filesystem::current_path(currentWorkspace_->getPath());
        
        // Add all changes
        currentWorkspace_->runCommand("git add .");
        
        // Commit
        std::string cmd = "git commit -m \"" + commitMsg.toStdString() + "\"";
        std::string result = currentWorkspace_->runCommand(cmd);
        
        std::filesystem::current_path(cwd);
        
        if (result.find("nothing to commit") != std::string::npos) {
            QMessageBox::information(this, "Nothing to Commit", "No changes to commit.");
            return;
        }
        
        if (result.find("fatal") == std::string::npos) {
            // Now push
            gitPush();
        } else {
            QMessageBox::warning(this, "Commit Failed", "Failed to commit changes.\n" + QString::fromStdString(result));
        }
    }
}

void MainWindow::editMakefile() {
    if (!currentWorkspace_) {
        QMessageBox::warning(this, "No Workspace", "Please select a workspace first.");
        return;
    }
    
    QString makefilePath;
    QDir workspaceDir(QString::fromStdString(currentWorkspace_->getPath()));
    
    // Look for Makefile in common locations
    QStringList makefileNames = {"Makefile", "makefile", "GNUmakefile"};
    for (const QString& name : makefileNames) {
        if (workspaceDir.exists(name)) {
            makefilePath = workspaceDir.absoluteFilePath(name);
            break;
        }
    }
    
    // Check build directory for generated Makefiles
    if (makefilePath.isEmpty()) {
        QString buildDir = QString::fromStdString(currentWorkspace_->getBuildDirectory());
        QDir buildDirObj(buildDir);
        for (const QString& name : makefileNames) {
            if (buildDirObj.exists(name)) {
                makefilePath = buildDirObj.absoluteFilePath(name);
                break;
            }
        }
    }
    
    if (makefilePath.isEmpty()) {
        QMessageBox::information(this, "No Makefile", "No Makefile found in the workspace.");
        return;
    }
    
    // Use the default system editor or code editor
    QProcess* editorProcess = new QProcess(this);
    QString command;
    QStringList args;
    
    // Try to use VS Code first, then gedit as fallback
    if (QProcess::execute("which", QStringList() << "code") == 0) {
        command = "code";
        args << makefilePath;
    } else if (QProcess::execute("which", QStringList() << "gedit") == 0) {
        command = "gedit";
        args << makefilePath;
    } else {
        command = "xdg-open";
        args << makefilePath;
    }
    
    editorProcess->start(command, args);
    if (!editorProcess->waitForStarted()) {
        QMessageBox::warning(this, "Editor Failed", "Failed to open Makefile in editor.");
    }
}

void MainWindow::setBuildSystem() {
    if (!currentWorkspace_) return;
    
    QString buildSystem = buildSystemCombo_->currentText();
    // You can store this preference in workspace settings or globally
    // For now, it affects the next build command
    
    infoDisplay_->append(QString("Build system set to: %1").arg(buildSystem));
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Save workspaces before closing
    wm_.saveToFile();
    
    // Clean up build process if running
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        buildProcess_->terminate();
        if (!buildProcess_->waitForFinished(3000)) {
            buildProcess_->kill();
        }
    }
    
    // Accept the close event
    event->accept();
}

void MainWindow::discoverScripts() {
    availableScripts_.clear();
    scriptCombo_->clear();
    
    // Look for scripts in the scripts/ directory relative to the application
    QString appDir = QCoreApplication::applicationDirPath();
    QString scriptsPath = QDir(appDir).absoluteFilePath("../scripts");
    QDir scriptsDir(scriptsPath);
    
    if (!scriptsDir.exists()) {
        scriptCombo_->addItem("No scripts directory found");
        runScriptButton_->setEnabled(false);
        return;
    }
    
    // Find all .sh files in the scripts directory
    QStringList filters;
    filters << "*.sh";
    QFileInfoList scriptFiles = scriptsDir.entryInfoList(filters, QDir::Files | QDir::Readable);
    
    if (scriptFiles.isEmpty()) {
        scriptCombo_->addItem("No scripts found");
        runScriptButton_->setEnabled(false);
        return;
    }
    
    // Populate the combo box with script names
    for (const QFileInfo &fileInfo : scriptFiles) {
        QString scriptName = fileInfo.baseName();
        availableScripts_.append(fileInfo.absoluteFilePath());
        scriptCombo_->addItem(scriptName);
    }
    
    runScriptButton_->setEnabled(true);
}

void MainWindow::runScript() {
    if (scriptCombo_->currentIndex() < 0 || scriptCombo_->currentIndex() >= availableScripts_.size()) {
        QMessageBox::warning(this, "No Script Selected", "Please select a script to run.");
        return;
    }
    
    QString scriptPath = availableScripts_[scriptCombo_->currentIndex()];
    QString scriptName = scriptCombo_->currentText();
    
    // Confirm script execution
    int ret = QMessageBox::question(this, "Run Script", 
                                  QString("Are you sure you want to run the script '%1'?").arg(scriptName),
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    // Clear build output and show script execution
    buildOutput_->clear();
    buildOutput_->append(QString("=== Running script: %1 ===").arg(scriptName));
    buildOutput_->append(QString("Script path: %1").arg(scriptPath));
    buildOutput_->append(""); // Empty line
    
    // Create process to run the script
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "Process Running", "Another process is already running. Please wait for it to finish.");
        return;
    }
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onScriptFinished);
    connect(buildProcess_, &QProcess::readyReadStandardOutput,
            this, &MainWindow::onScriptOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError,
            this, &MainWindow::onScriptOutput);
    
    // Show progress
    buildProgress_->setVisible(true);
    buildProgress_->setRange(0, 0); // Indeterminate progress
    
    // Start the script
    buildProcess_->setWorkingDirectory(QDir::currentPath());
    buildProcess_->start("bash", QStringList() << scriptPath);
    
    if (!buildProcess_->waitForStarted()) {
        QMessageBox::critical(this, "Script Failed", "Failed to start the script.");
        buildProgress_->setVisible(false);
        buildProcess_->deleteLater();
        buildProcess_ = nullptr;
    }
}

void MainWindow::installSystemWide() {
    // Confirm system-wide installation
    int ret = QMessageBox::question(this, "System-Wide Installation", 
                                  "This will install C++ Workspace Manager system-wide. "
                                  "You may be prompted for your password. Continue?",
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    // Clear build output and show installation process
    buildOutput_->clear();
    buildOutput_->append("=== Installing C++ Workspace Manager System-Wide ===");
    buildOutput_->append("");
    
    // Create process to run system-wide installation
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "Process Running", "Another process is already running. Please wait for it to finish.");
        return;
    }
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onInstallFinished);
    connect(buildProcess_, &QProcess::readyReadStandardOutput,
            this, &MainWindow::onScriptOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError,
            this, &MainWindow::onScriptOutput);
    
    // Show progress
    buildProgress_->setVisible(true);
    buildProgress_->setRange(0, 0); // Indeterminate progress
    
    // Check if system-wide install script exists, create it if not
    QString installScript = QDir::currentPath() + "/scripts/install-system-wide.sh";
    QFile scriptFile(installScript);
    if (!scriptFile.exists()) {
        createSystemWideInstallScript(installScript);
    }
    
    // Start the installation script
    buildProcess_->setWorkingDirectory(QDir::currentPath());
    buildProcess_->start("bash", QStringList() << installScript);
    
    if (!buildProcess_->waitForStarted()) {
        QMessageBox::critical(this, "Installation Failed", "Failed to start the installation script.");
        buildProgress_->setVisible(false);
        buildProcess_->deleteLater();
        buildProcess_ = nullptr;
    }
}

void MainWindow::onScriptFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    buildProgress_->setVisible(false);
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        buildOutput_->append("\n=== Script completed successfully! ===");
        QMessageBox::information(this, "Script Complete", "Script executed successfully!");
    } else {
        buildOutput_->append(QString("\n=== Script failed with exit code: %1 ===").arg(exitCode));
        QMessageBox::warning(this, "Script Failed", "Script execution failed. Check the output for details.");
    }
    
    buildProcess_->deleteLater();
    buildProcess_ = nullptr;
}

void MainWindow::onInstallFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    buildProgress_->setVisible(false);
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        buildOutput_->append("\n=== System-wide installation completed successfully! ===");
        QMessageBox::information(this, "Installation Complete", 
                               "C++ Workspace Manager has been installed system-wide!\n"
                               "You can now run 'cppm' from anywhere in the terminal.");
    } else {
        buildOutput_->append(QString("\n=== Installation failed with exit code: %1 ===").arg(exitCode));
        QMessageBox::warning(this, "Installation Failed", "System-wide installation failed. Check the output for details.");
    }
    
    buildProcess_->deleteLater();
    buildProcess_ = nullptr;
}

void MainWindow::onScriptOutput() {
    if (buildProcess_) {
        QByteArray data = buildProcess_->readAllStandardOutput();
        QString output = QString::fromLocal8Bit(data);
        buildOutput_->append(output);
        
        // Also read stderr
        data = buildProcess_->readAllStandardError();
        if (!data.isEmpty()) {
            QString errorOutput = QString::fromLocal8Bit(data);
            buildOutput_->append(QString("<font color='red'>%1</font>").arg(errorOutput));
        }
    }
}

void MainWindow::createSystemWideInstallScript(const QString& scriptPath) {
    QFile file(scriptPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "#!/bin/bash\n";
        out << "\n";
        out << "# C++ Workspace Manager - System-Wide Installation Script\n";
        out << "# This script installs cppm system-wide\n";
        out << "\n";
        out << "set -e\n";
        out << "\n";
        out << "echo \"Installing C++ Workspace Manager system-wide...\"\n";
        out << "\n";
        out << "# Check if executable exists\n";
        out << "if [ ! -f \"build/cppm\" ]; then\n";
        out << "    echo \"Error: build/cppm not found. Please build the project first.\"\n";
        out << "    exit 1\n";
        out << "fi\n";
        out << "\n";
        out << "# Install executable to /usr/local/bin\n";
        out << "echo \"Installing executable to /usr/local/bin...\"\n";
        out << "sudo cp build/cppm /usr/local/bin/\n";
        out << "sudo chmod +x /usr/local/bin/cppm\n";
        out << "\n";
        out << "# Create desktop entry\n";
        out << "echo \"Creating desktop entry...\"\n";
        out << "cat > ~/.local/share/applications/cppm.desktop << EOF\n";
        out << "[Desktop Entry]\n";
        out << "Name=C++ Workspace Manager\n";
        out << "Comment=Manage C++ development workspaces\n";
        out << "Exec=/usr/local/bin/cppm\n";
        out << "Icon=applications-development\n";
        out << "Terminal=false\n";
        out << "Type=Application\n";
        out << "Categories=Development;IDE;\n";
        out << "EOF\n";
        out << "\n";
        out << "# Update desktop database\n";
        out << "if command -v update-desktop-database &> /dev/null; then\n";
        out << "    update-desktop-database ~/.local/share/applications/\n";
        out << "fi\n";
        out << "\n";
        out << "echo \"Installation completed successfully!\"\n";
        out << "echo \"You can now run 'cppm' from anywhere in the terminal.\"\n";
        out << "echo \"You can also find 'C++ Workspace Manager' in your application menu.\"\n";
        
        file.close();
        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                           QFile::ReadGroup | QFile::ExeGroup |
                           QFile::ReadOther | QFile::ExeOther);
    }
}

void MainWindow::onCloneFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString& repoName, const QString& clonePath) {
    buildProgress_->setVisible(false);
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        buildOutput_->append("\n=== Clone completed successfully! ===");
        
        // Automatically add the cloned repository as a workspace
        wm_.addWorkspace(repoName.toStdString(), clonePath.toStdString());
        workspaceList_->addItem(repoName + ": " + clonePath);
        
        // Select the newly added workspace
        workspaceList_->setCurrentRow(workspaceList_->count() - 1);
        currentWorkspaceName_ = repoName;
        currentWorkspace_ = wm_.getWorkspace(repoName.toStdString());
        
        QMessageBox::information(this, "Clone Complete", 
                               QString("Repository cloned successfully!\nAdded '%1' to workspaces.").arg(repoName));
    } else {
        buildOutput_->append(QString("\n=== Clone failed with exit code: %1 ===").arg(exitCode));
        QMessageBox::warning(this, "Clone Failed", "Repository clone failed. Check the output for details.");
    }
    
    buildProcess_->deleteLater();
    buildProcess_ = nullptr;
}

void MainWindow::renameRepository() {
    if (!currentWorkspace_) {
        QMessageBox::warning(this, "No Workspace", "Please select a workspace first.");
        return;
    }
    
    QString currentPath = QString::fromStdString(currentWorkspace_->getPath());
    QDir workspaceDir(currentPath);
    
    // Check if this is a git repository
    if (!QDir(currentPath + "/.git").exists()) {
        QMessageBox::warning(this, "Not a Git Repository", "The selected workspace is not a git repository.");
        return;
    }
    
    QString newName = QInputDialog::getText(this, "Rename Repository", 
                                          "New repository name:", 
                                          QLineEdit::Normal, 
                                          workspaceDir.dirName());
    
    if (newName.isEmpty() || newName == workspaceDir.dirName()) {
        return;
    }
    
    // Validate repository name
    QRegularExpression namePattern("^[a-zA-Z0-9._-]+$");
    if (!namePattern.match(newName).hasMatch()) {
        QMessageBox::warning(this, "Invalid Name", "Repository name can only contain letters, numbers, dots, underscores, and hyphens.");
        return;
    }
    
    QString parentDir = workspaceDir.absolutePath();
    parentDir = parentDir.left(parentDir.lastIndexOf('/'));
    QString newPath = parentDir + "/" + newName;
    
    // Check if target directory exists
    if (QDir(newPath).exists()) {
        QMessageBox::warning(this, "Directory Exists", QString("Directory '%1' already exists.").arg(newPath));
        return;
    }
    
    // Rename the directory
    if (workspaceDir.rename(currentPath, newPath)) {
        // Update workspace in manager
        wm_.removeWorkspace(currentWorkspaceName_.toStdString());
        wm_.addWorkspace(newName.toStdString(), newPath.toStdString());
        
        // Update UI
        currentWorkspaceName_ = newName;
        refreshWorkspace();
        
        QMessageBox::information(this, "Rename Successful", 
                               QString("Repository renamed to '%1' successfully!").arg(newName));
    } else {
        QMessageBox::critical(this, "Rename Failed", "Failed to rename the repository directory.");
    }
}

void MainWindow::createGithubRelease() {
    if (!currentWorkspace_) {
        QMessageBox::warning(this, "No Workspace", "Please select a workspace first.");
        return;
    }
    
    QString currentPath = QString::fromStdString(currentWorkspace_->getPath());
    
    // Check if this is a git repository
    if (!QDir(currentPath + "/.git").exists()) {
        QMessageBox::warning(this, "Not a Git Repository", "The selected workspace is not a git repository.");
        return;
    }
    
    // Get current version
    QString version = getCurrentVersion(currentWorkspace_);
    
    QString releaseVersion = QInputDialog::getText(this, "Create GitHub Release", 
                                                 "Release version:", 
                                                 QLineEdit::Normal, 
                                                 version.isEmpty() ? "v1.0.0" : version);
    
    if (releaseVersion.isEmpty()) {
        return;
    }
    
    QString releaseNotes = QInputDialog::getMultiLineText(this, "Create GitHub Release",
                                                        "Release notes (optional):");
    
    // Show progress and create release
    buildOutput_->clear();
    buildOutput_->append(QString("=== Creating GitHub release: %1 ===").arg(releaseVersion));
    buildOutput_->append("");
    
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "Process Running", "Another process is already running. Please wait for it to finish.");
        return;
    }
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onGithubActionFinished);
    connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onScriptOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onScriptOutput);
    
    buildProgress_->setVisible(true);
    buildProgress_->setRange(0, 0);
    
    // Create a tag first, then try to create release using gh CLI if available
    buildProcess_->setWorkingDirectory(currentPath);
    
    // First create and push the tag
    QStringList tagArgs;
    tagArgs << "tag" << releaseVersion;
    if (!releaseNotes.isEmpty()) {
        tagArgs << "-m" << releaseNotes;
    }
    
    buildProcess_->start("git", tagArgs);
    
    if (!buildProcess_->waitForStarted()) {
        QMessageBox::critical(this, "Release Failed", "Failed to create git tag.");
        buildProgress_->setVisible(false);
        buildProcess_->deleteLater();
        buildProcess_ = nullptr;
    }
}

void MainWindow::openGithubRepo() {
    if (!currentWorkspace_) {
        QMessageBox::warning(this, "No Workspace", "Please select a workspace first.");
        return;
    }
    
    QString currentPath = QString::fromStdString(currentWorkspace_->getPath());
    
    // Check if this is a git repository
    if (!QDir(currentPath + "/.git").exists()) {
        QMessageBox::warning(this, "Not a Git Repository", "The selected workspace is not a git repository.");
        return;
    }
    
    // Get remote URL
    QProcess process;
    process.setWorkingDirectory(currentPath);
    process.start("git", QStringList() << "remote" << "get-url" << "origin");
    process.waitForFinished();
    
    QString remoteUrl = process.readAllStandardOutput().trimmed();
    
    if (remoteUrl.isEmpty()) {
        QMessageBox::warning(this, "No Remote", "No GitHub remote found for this repository.");
        return;
    }
    
    // Convert SSH URL to HTTPS URL if needed
    if (remoteUrl.startsWith("git@github.com:")) {
        remoteUrl = remoteUrl.replace("git@github.com:", "https://github.com/");
        remoteUrl = remoteUrl.replace(".git", "");
    }
    
    // Open in default browser
    QDesktopServices::openUrl(QUrl(remoteUrl));
}

void MainWindow::viewGithubIssues() {
    if (!currentWorkspace_) {
        QMessageBox::warning(this, "No Workspace", "Please select a workspace first.");
        return;
    }
    
    QString currentPath = QString::fromStdString(currentWorkspace_->getPath());
    
    // Check if this is a git repository
    if (!QDir(currentPath + "/.git").exists()) {
        QMessageBox::warning(this, "Not a Git Repository", "The selected workspace is not a git repository.");
        return;
    }
    
    // Get remote URL
    QProcess process;
    process.setWorkingDirectory(currentPath);
    process.start("git", QStringList() << "remote" << "get-url" << "origin");
    process.waitForFinished();
    
    QString remoteUrl = process.readAllStandardOutput().trimmed();
    
    if (remoteUrl.isEmpty()) {
        QMessageBox::warning(this, "No Remote", "No GitHub remote found for this repository.");
        return;
    }
    
    // Convert SSH URL to HTTPS URL if needed
    if (remoteUrl.startsWith("git@github.com:")) {
        remoteUrl = remoteUrl.replace("git@github.com:", "https://github.com/");
        remoteUrl = remoteUrl.replace(".git", "");
    }
    
    // Add /issues to the URL
    remoteUrl += "/issues";
    
    // Open in default browser
    QDesktopServices::openUrl(QUrl(remoteUrl));
}

void MainWindow::onGithubActionFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    buildProgress_->setVisible(false);
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        buildOutput_->append("\n=== GitHub action completed successfully! ===");
        QMessageBox::information(this, "Action Complete", "GitHub action completed successfully!");
    } else {
        buildOutput_->append(QString("\n=== GitHub action failed with exit code: %1 ===").arg(exitCode));
        QMessageBox::warning(this, "Action Failed", "GitHub action failed. Check the output for details.");
    }
    
    buildProcess_->deleteLater();
    buildProcess_ = nullptr;
}

void MainWindow::authenticateGithub() {
    bool ok;
    QString token = QInputDialog::getText(this, "GitHub Authentication", 
                                        "Enter your GitHub Personal Access Token:\n"
                                        "(You can create one at: https://github.com/settings/tokens)\n"
                                        "Required scopes: repo, user", 
                                        QLineEdit::Password, "", &ok);
    
    if (!ok || token.isEmpty()) {
        return;
    }
    
    // Verify the token by making a simple API call
    buildOutput_->clear();
    buildOutput_->append("=== Verifying GitHub authentication ===");
    buildOutput_->append("");
    
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "Process Running", "Another process is already running. Please wait for it to finish.");
        return;
    }
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, token](int exitCode, QProcess::ExitStatus exitStatus) {
                onGithubAuthFinished(exitCode, exitStatus, token);
            });
    connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onScriptOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onScriptOutput);
    
    buildProgress_->setVisible(true);
    buildProgress_->setRange(0, 0);
    
    // Use curl to verify the token
    QStringList args;
    args << "-H" << QString("Authorization: token %1").arg(token)
         << "-H" << "Accept: application/vnd.github.v3+json"
         << "https://api.github.com/user";
    
    buildProcess_->start("curl", args);
    
    if (!buildProcess_->waitForStarted()) {
        QMessageBox::critical(this, "Authentication Failed", "Failed to verify GitHub token. Make sure curl is installed.");
        buildProgress_->setVisible(false);
        buildProcess_->deleteLater();
        buildProcess_ = nullptr;
    }
}

void MainWindow::openGithubTokenPage() {
    QString url = "https://github.com/settings/tokens/new?scopes=repo,user&description=C%2B%2B%20Projects%20Manager";
    
    QMessageBox::information(this, "Generate GitHub Token", 
                           "This will open GitHub's Personal Access Token creation page.\n\n"
                           "Please:\n"
                           "1. Generate a new token with 'repo' and 'user' scopes\n"
                           "2. Copy the generated token\n"
                           "3. Use 'GitHub Auth' button to enter the token");
    
    if (!QDesktopServices::openUrl(QUrl(url))) {
        QMessageBox::warning(this, "Error", "Failed to open browser. Please manually visit:\n" + url);
    }
}

void MainWindow::browseGithubRepos() {
    if (!isGithubAuthenticated_) {
        QMessageBox::warning(this, "Not Authenticated", "Please authenticate with GitHub first.");
        return;
    }
    
    buildOutput_->clear();
    buildOutput_->append("=== Fetching your GitHub repositories ===");
    buildOutput_->append("");
    
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "Process Running", "Another process is already running. Please wait for it to finish.");
        return;
    }
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onBrowseReposFinished);
    connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onScriptOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onScriptOutput);
    
    buildProgress_->setVisible(true);
    buildProgress_->setRange(0, 0);
    
    // Fetch user's repositories
    QStringList args;
    args << "-H" << QString("Authorization: token %1").arg(githubToken_)
         << "-H" << "Accept: application/vnd.github.v3+json"
         << "https://api.github.com/user/repos?per_page=100&sort=updated";
    
    buildProcess_->start("curl", args);
    
    if (!buildProcess_->waitForStarted()) {
        QMessageBox::critical(this, "Failed", "Failed to fetch repositories. Make sure curl is installed.");
        buildProgress_->setVisible(false);
        buildProcess_->deleteLater();
        buildProcess_ = nullptr;
    }
}

void MainWindow::createGithubRepo() {
    if (!isGithubAuthenticated_) {
        QMessageBox::warning(this, "Not Authenticated", "Please authenticate with GitHub first.");
        return;
    }
    
    bool ok;
    QString repoName = QInputDialog::getText(this, "Create GitHub Repository",
                                           "Repository name:", QLineEdit::Normal, "", &ok);
    
    if (!ok || repoName.isEmpty()) {
        return;
    }
    
    QString description = QInputDialog::getText(this, "Create GitHub Repository",
                                              "Description (optional):", QLineEdit::Normal, "", &ok);
    
    if (!ok) {
        return;
    }
    
    bool isPrivate = QMessageBox::question(this, "Repository Visibility",
                                         "Make repository private?",
                                         QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
    
    buildOutput_->clear();
    buildOutput_->append(QString("=== Creating GitHub repository: %1 ===").arg(repoName));
    buildOutput_->append("");
    
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "Process Running", "Another process is already running. Please wait for it to finish.");
        return;
    }
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, repoName](int exitCode, QProcess::ExitStatus exitStatus) {
                onCreateRepoFinished(exitCode, exitStatus, repoName);
            });
    connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onScriptOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onScriptOutput);
    
    buildProgress_->setVisible(true);
    buildProgress_->setRange(0, 0);
    
    // Create JSON payload
    QString jsonData = QString("{"
                              "\"name\":\"%1\","
                              "\"description\":\"%2\","
                              "\"private\":%3,"
                              "\"auto_init\":true"
                              "}")
                       .arg(repoName)
                       .arg(description.isEmpty() ? "" : description)
                       .arg(isPrivate ? "true" : "false");
    
    // Use curl to create repository
    QStringList args;
    args << "-X" << "POST"
         << "-H" << QString("Authorization: token %1").arg(githubToken_)
         << "-H" << "Accept: application/vnd.github.v3+json"
         << "-H" << "Content-Type: application/json"
         << "--data" << jsonData
         << "https://api.github.com/user/repos";
    
    buildProcess_->start("curl", args);
    
    if (!buildProcess_->waitForStarted()) {
        QMessageBox::critical(this, "Failed", "Failed to create repository. Make sure curl is installed.");
        buildProgress_->setVisible(false);
        buildProcess_->deleteLater();
        buildProcess_ = nullptr;
    }
}

void MainWindow::onGithubAuthFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString& token) {
    buildProgress_->setVisible(false);
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        // Parse the response to get username
        QString output = buildProcess_->readAllStandardOutput();
        
        // Simple JSON parsing to extract username
        QRegularExpression usernamePattern("\"login\"\\s*:\\s*\"([^\"]+)\"");
        QRegularExpressionMatch match = usernamePattern.match(output);
        
        if (match.hasMatch()) {
            githubToken_ = token;
            githubUsername_ = match.captured(1);
            isGithubAuthenticated_ = true;
            
            buildOutput_->append(QString("\\n=== Authentication successful! ==="));
            buildOutput_->append(QString("Logged in as: %1").arg(githubUsername_));
            
            QMessageBox::information(this, "Authentication Successful", 
                                   QString("Successfully authenticated as: %1").arg(githubUsername_));
            
            // Update button text to show authenticated state
            githubAuthButton_->setText(QString("Auth: %1").arg(githubUsername_));
            browseReposButton_->setEnabled(true);
            createRepoButton_->setEnabled(true);
        } else {
            buildOutput_->append("\\n=== Authentication failed: Invalid response ===");
            QMessageBox::warning(this, "Authentication Failed", "Invalid response from GitHub. Please check your token.");
        }
    } else {
        buildOutput_->append(QString("\\n=== Authentication failed with exit code: %1 ===").arg(exitCode));
        QMessageBox::warning(this, "Authentication Failed", "Failed to authenticate with GitHub. Please check your token and internet connection.");
    }
    
    buildProcess_->deleteLater();
    buildProcess_ = nullptr;
}

void MainWindow::onBrowseReposFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    buildProgress_->setVisible(false);
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        QString output = buildProcess_->readAllStandardOutput();
        
        // Parse repository information (name, description, private status, language)
        QList<MainWindow::RepoInfo> repositories;
        
        // Simpler parsing approach - parse line by line for key fields
        QStringList lines = output.split('\n');
        MainWindow::RepoInfo currentRepo;
        bool inRepo = false;
        
        for (const QString& line : lines) {
            QString trimmedLine = line.trimmed();
            
            if (trimmedLine.contains("\"full_name\":")) {
                if (inRepo && !currentRepo.fullName.isEmpty()) {
                    repositories.append(currentRepo);
                }
                currentRepo = MainWindow::RepoInfo();
                inRepo = true;
                
                QRegularExpression namePattern("\"full_name\"\\s*:\\s*\"([^\"]+)\"");
                QRegularExpressionMatch match = namePattern.match(trimmedLine);
                if (match.hasMatch()) {
                    currentRepo.fullName = match.captured(1);
                }
            }
            else if (inRepo && trimmedLine.contains("\"description\":")) {
                QRegularExpression descPattern("\"description\"\\s*:\\s*\"([^\"]*)\"");
                QRegularExpressionMatch match = descPattern.match(trimmedLine);
                if (match.hasMatch()) {
                    currentRepo.description = match.captured(1);
                    if (currentRepo.description == "null") currentRepo.description = "";
                }
            }
            else if (inRepo && trimmedLine.contains("\"language\":")) {
                QRegularExpression langPattern("\"language\"\\s*:\\s*\"([^\"]*)\"");
                QRegularExpressionMatch match = langPattern.match(trimmedLine);
                if (match.hasMatch()) {
                    currentRepo.language = match.captured(1);
                    if (currentRepo.language == "null") currentRepo.language = "";
                }
            }
            else if (inRepo && trimmedLine.contains("\"private\":")) {
                currentRepo.isPrivate = trimmedLine.contains("true");
            }
            else if (inRepo && trimmedLine.contains("\"updated_at\":")) {
                QRegularExpression updatePattern("\"updated_at\"\\s*:\\s*\"([^\"]+)\"");
                QRegularExpressionMatch match = updatePattern.match(trimmedLine);
                if (match.hasMatch()) {
                    currentRepo.updatedAt = match.captured(1);
                }
            }
        }
        
        // Don't forget the last repository
        if (inRepo && !currentRepo.fullName.isEmpty()) {
            repositories.append(currentRepo);
        }
        
        if (!repositories.isEmpty()) {
            // Create custom dialog to show repository details
            showRepositoryDialog(repositories);
        } else {
            buildOutput_->append("\\n=== No repositories found ===");
            QMessageBox::information(this, "No Repositories", "No repositories found in your GitHub account.");
        }
    } else {
        buildOutput_->append(QString("\\n=== Failed to fetch repositories with exit code: %1 ===").arg(exitCode));
        QMessageBox::warning(this, "Failed", "Failed to fetch repositories from GitHub.");
    }
    
    buildProcess_->deleteLater();
    buildProcess_ = nullptr;
}

void MainWindow::onCreateRepoFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString& repoName) {
    buildProgress_->setVisible(false);
    
    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        QString output = buildProcess_->readAllStandardOutput();
        
        // Check if creation was successful
        if (output.contains("\"full_name\"")) {
            buildOutput_->append("\\n=== Repository created successfully! ===");
            
            QString cloneUrl = QString("https://github.com/%1/%2.git").arg(githubUsername_).arg(repoName);
            
            int ret = QMessageBox::question(this, "Repository Created",
                                          QString("Repository '%1' created successfully!\\n\\nWould you like to clone it now?").arg(repoName),
                                          QMessageBox::Yes | QMessageBox::No);
            
            if (ret == QMessageBox::Yes) {
                QString baseDir = QFileDialog::getExistingDirectory(this, "Select Directory to Clone Into", QDir::homePath());
                if (!baseDir.isEmpty()) {
                    QString clonePath = baseDir + "/" + repoName;
                    startCloneProcess(cloneUrl, repoName, clonePath);
                }
            }
        } else {
            buildOutput_->append("\\n=== Repository creation failed ===");
            QMessageBox::warning(this, "Creation Failed", "Failed to create repository. It might already exist.");
        }
    } else {
        buildOutput_->append(QString("\\n=== Repository creation failed with exit code: %1 ===").arg(exitCode));
        QMessageBox::warning(this, "Creation Failed", "Failed to create repository on GitHub.");
    }
    
    buildProcess_->deleteLater();
    buildProcess_ = nullptr;
}

void MainWindow::startCloneProcess(const QString& repoUrl, const QString& repoName, const QString& clonePath) {
    // Check if directory already exists
    if (QDir(clonePath).exists()) {
        int ret = QMessageBox::question(this, "Directory Exists", 
                                      QString("Directory '%1' already exists. Continue anyway?").arg(clonePath),
                                      QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    // Show progress and start clone process
    buildOutput_->clear();
    buildOutput_->append(QString("=== Cloning repository: %1 ===").arg(repoUrl));
    buildOutput_->append(QString("Destination: %1").arg(clonePath));
    buildOutput_->append("");
    
    if (buildProcess_ && buildProcess_->state() != QProcess::NotRunning) {
        QMessageBox::warning(this, "Process Running", "Another process is already running. Please wait for it to finish.");
        return;
    }
    
    buildProcess_ = new QProcess(this);
    connect(buildProcess_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, repoName, clonePath](int exitCode, QProcess::ExitStatus exitStatus) {
                onCloneFinished(exitCode, exitStatus, repoName, clonePath);
            });
    connect(buildProcess_, &QProcess::readyReadStandardOutput, this, &MainWindow::onScriptOutput);
    connect(buildProcess_, &QProcess::readyReadStandardError, this, &MainWindow::onScriptOutput);
    
    buildProgress_->setVisible(true);
    buildProgress_->setRange(0, 0); // Indeterminate progress
    
    // Start git clone
    QString baseDir = QFileInfo(clonePath).absolutePath();
    buildProcess_->setWorkingDirectory(baseDir);
    buildProcess_->start("git", QStringList() << "clone" << repoUrl << repoName);
    
    if (!buildProcess_->waitForStarted()) {
        QMessageBox::critical(this, "Clone Failed", "Failed to start git clone process.");
        buildProgress_->setVisible(false);
        buildProcess_->deleteLater();
        buildProcess_ = nullptr;
    }
}

void MainWindow::showRepositoryDialog(const QList<RepoInfo>& repositories) {
    QDialog dialog(this);
    dialog.setWindowTitle(QString("GitHub Repositories (%1 found)").arg(repositories.size()));
    dialog.resize(800, 600);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    // Info label
    QLabel* infoLabel = new QLabel(QString("Signed in as: %1\\nFound %2 repositories (sorted by last updated)")
                                   .arg(githubUsername_).arg(repositories.size()));
    layout->addWidget(infoLabel);
    
    // Repository list
    QListWidget* repoList = new QListWidget();
    repoList->setAlternatingRowColors(true);
    
    for (const auto& repo : repositories) {
        // Format the display text
        QString displayText = QString("%1%2")
                             .arg(repo.fullName)
                             .arg(repo.isPrivate ? " 🔒" : "");
        
        if (!repo.language.isEmpty()) {
            displayText += QString(" [%1]").arg(repo.language);
        }
        
        if (!repo.description.isEmpty()) {
            displayText += QString("\\n    %1").arg(repo.description);
        }
        
        // Extract date for display
        if (!repo.updatedAt.isEmpty()) {
            QDateTime dateTime = QDateTime::fromString(repo.updatedAt, Qt::ISODate);
            if (dateTime.isValid()) {
                displayText += QString("\\n    Updated: %1").arg(dateTime.toString("MMM dd, yyyy"));
            }
        }
        
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, repo.fullName); // Store full name for cloning
        repoList->addItem(item);
    }
    
    layout->addWidget(repoList);
    
    // Buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox();
    QPushButton* cloneButton = buttonBox->addButton("Clone Selected", QDialogButtonBox::AcceptRole);
    QPushButton* cancelButton = buttonBox->addButton("Cancel", QDialogButtonBox::RejectRole);
    
    cloneButton->setEnabled(false);
    
    // Enable clone button when an item is selected
    connect(repoList, &QListWidget::itemSelectionChanged, [cloneButton, repoList]() {
        cloneButton->setEnabled(repoList->currentItem() != nullptr);
    });
    
    // Double-click to clone
    connect(repoList, &QListWidget::itemDoubleClicked, [&dialog]() {
        dialog.accept();
    });
    
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    layout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted && repoList->currentItem()) {
        QString selectedRepo = repoList->currentItem()->data(Qt::UserRole).toString();
        
        if (!selectedRepo.isEmpty()) {
            QString cloneUrl = QString("https://github.com/%1.git").arg(selectedRepo);
            
            // Choose destination directory
            QString baseDir = QFileDialog::getExistingDirectory(this, "Select Directory to Clone Into", QDir::homePath());
            if (!baseDir.isEmpty()) {
                // Extract repo name
                QString repoName = selectedRepo.split('/').last();
                QString clonePath = baseDir + "/" + repoName;
                
                // Start clone process
                startCloneProcess(cloneUrl, repoName, clonePath);
            }
        }
    }
}

#include "moc_MainWindow.cpp"
