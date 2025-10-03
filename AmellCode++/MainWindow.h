#pragma once

#include <QMainWindow>
#include <QProcess>

class Editor;
class QTreeView;
class QFileSystemModel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void buildProject();
    void runProject();
    void onBuildReadyRead();
    void onBuildFinished(int exitCode, QProcess::ExitStatus status);

private:
    void createMenus();
    void createToolbar();
    void createDocks();
    void applyBluePalette();

    Editor *m_editor;
    QTreeView *m_projectTree;
    QFileSystemModel *m_fsModel;
    QProcess *m_buildProcess;
};
