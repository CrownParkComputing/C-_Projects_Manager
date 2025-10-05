#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QLabel>
#include <QSplitter>
#include <QProcess>
#include <QProgressBar>
#include "WorkspaceManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    void addWorkspace();
    void addLocalWorkspace();
    void cloneGithubRepo();
    void removeWorkspace();
    void selectWorkspace(QListWidgetItem* item);
    void refreshWorkspace();

    // Build management actions
    void buildWorkspace();
    void cleanWorkspace();
    void runWorkspace();
    void editMakefile();
    void setBuildSystem();

    // Git versioning actions
    void gitMajorVersion();
    void gitMinorVersion();
    void gitPatchVersion();
    void gitPush();
    void gitCommitAndPush();
    void renameRepository();
    void createGithubRelease();
    void openGithubRepo();
    void viewGithubIssues();
    void authenticateGithub();
    void openGithubTokenPage();
    void browseGithubRepos();
    void createGithubRepo();

    // Script management actions
    void runScript();
    void installSystemWide();

    // Build process slots
    void onBuildFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onCmakeFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onBuildOutput();
    void onScriptFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onInstallFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onScriptOutput();
    void onCloneFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString& repoName, const QString& clonePath);
    void onGithubActionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onGithubAuthFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString& token);
    void onBrowseReposFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onCreateRepoFinished(int exitCode, QProcess::ExitStatus exitStatus, const QString& repoName);

private:
    void setupUI();
    void displayWorkspaceInfo(Workspace* ws);
    void updateActionButtons();
    QString getCurrentVersion(Workspace* ws);
    QString incrementVersion(const QString& version, int type); // 0=patch, 1=minor, 2=major
    void createVersionTag(const QString& version);
    void startMakeBuild();
    void populateScriptList();
    void discoverScripts();
    void createSystemWideInstallScript(const QString& scriptPath);
    void startCloneProcess(const QString& repoUrl, const QString& repoName, const QString& clonePath);
    
    struct RepoInfo {
        QString fullName;
        QString description;
        QString language;
        bool isPrivate;
        QString updatedAt;
    };
    void showRepositoryDialog(const QList<RepoInfo>& repositories);

    WorkspaceManager wm_;
    QListWidget* workspaceList_;
    QTextEdit* infoDisplay_;
    QPushButton* addLocalButton_;
    QPushButton* cloneGithubButton_;
    QPushButton* removeButton_;
    QPushButton* refreshButton_;
    
    // Current workspace tracking
    Workspace* currentWorkspace_;
    QString currentWorkspaceName_;
    
    // Build management buttons
    QGroupBox* buildGroup_;
    QPushButton* buildButton_;
    QPushButton* cleanButton_;
    QPushButton* runButton_;
    QPushButton* editMakefileButton_;
    QComboBox* buildSystemCombo_;
    QLabel* buildSystemLabel_;
    
    // Git versioning buttons
    QGroupBox* gitGroup_;
    QPushButton* majorButton_;
    QPushButton* minorButton_;
    QPushButton* patchButton_;
    QPushButton* pushButton_;
    QPushButton* commitPushButton_;
    QPushButton* renameRepoButton_;
    QPushButton* createReleaseButton_;
    QPushButton* openGithubButton_;
    QPushButton* viewIssuesButton_;
    QPushButton* githubAuthButton_;
    QPushButton* generateTokenButton_;
    QPushButton* browseReposButton_;
    QPushButton* createRepoButton_;
    QComboBox* branchCombo_;
    QLabel* versionLabel_;

    // Script management
    QGroupBox* scriptGroup_;
    QComboBox* scriptCombo_;
    QPushButton* runScriptButton_;
    QPushButton* installSystemWideButton_;
    QStringList availableScripts_;

    // GitHub authentication
    QString githubToken_;
    QString githubUsername_;
    bool isGithubAuthenticated_;
    QStringList userRepositories_;

    // Build progress
    QProcess* buildProcess_;
    QTextEdit* buildOutput_;
    QProgressBar* buildProgress_;
};

#endif // MAINWINDOW_H
