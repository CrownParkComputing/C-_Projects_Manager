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
#include <QRegularExpression>
#include <QLineEdit>
#include <QProgressBar>
#include <QCloseEvent>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), currentWorkspace_(nullptr), buildProcess_(nullptr), buildOutput_(nullptr), buildProgress_(nullptr) {
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

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Left side: workspace list and add button
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    addButton_ = new QPushButton("Add Workspace", leftWidget);
    leftLayout->addWidget(addButton_);

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
    cleanButton_ = new QPushButton("Clean");
    runButton_ = new QPushButton("Run");
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

    gitLayout->addWidget(pushButton_);
    gitLayout->addWidget(commitPushButton_);

    actionsLayout->addWidget(gitGroup_);

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
    connect(addButton_, &QPushButton::clicked, this, &MainWindow::addWorkspace);
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

    // Initially disable action buttons
    updateActionButtons();

    // Load existing workspaces
    auto workspaces = wm_.listWorkspaces();
    for (const auto& ws : workspaces) {
        workspaceList_->addItem(QString::fromStdString(ws));
    }
}

void MainWindow::addWorkspace() {
    QString name = QInputDialog::getText(this, "Add Workspace", "Workspace name:");
    if (!name.isEmpty()) {
        QString path = QFileDialog::getExistingDirectory(this, "Select Workspace Directory", QDir::homePath());
        if (!path.isEmpty()) {
            wm_.addWorkspace(name.toStdString(), path.toStdString());
            workspaceList_->addItem(name + ": " + path);
        }
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
    std::string name = text.left(colon).toStdString();
    currentWorkspace_ = wm_.getWorkspace(name);
    if (currentWorkspace_) {
        displayWorkspaceInfo(currentWorkspace_);
        updateActionButtons();
    }
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

    buildButton_->setEnabled(hasWorkspace);
    cleanButton_->setEnabled(hasWorkspace);
    runButton_->setEnabled(hasWorkspace);

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

#include "moc_MainWindow.cpp"
