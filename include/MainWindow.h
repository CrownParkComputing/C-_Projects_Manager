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

    // Build process slots
    void onBuildFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onCmakeFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onBuildOutput();

private:
    void setupUI();
    void displayWorkspaceInfo(Workspace* ws);
    void updateActionButtons();
    QString getCurrentVersion(Workspace* ws);
    QString incrementVersion(const QString& version, int type); // 0=patch, 1=minor, 2=major
    void createVersionTag(const QString& version);
    void startMakeBuild();

    WorkspaceManager wm_;
    QListWidget* workspaceList_;
    QTextEdit* infoDisplay_;
    QPushButton* addButton_;
    QPushButton* refreshButton_;
    
    // Current workspace tracking
    Workspace* currentWorkspace_;
    
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
    QComboBox* branchCombo_;
    QLabel* versionLabel_;

    // Build progress
    QProcess* buildProcess_;
    QTextEdit* buildOutput_;
    QProgressBar* buildProgress_;
};

#endif // MAINWINDOW_H
