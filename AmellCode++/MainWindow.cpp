#include "MainWindow.h"
#include "Editor.h"

#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeView>
#include <QDockWidget>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QColor>
#include <QPalette>
#include <QAction>
#include <QKeySequence>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_editor(new Editor(this)),
      m_projectTree(nullptr),
      m_fsModel(nullptr),
      m_buildProcess(nullptr) {
    setWindowTitle("AMELL-IDE");
    setCentralWidget(m_editor);
    resize(1100, 700);

    createMenus();
    createToolbar();
    createDocks();
    applyBluePalette();
}

MainWindow::~MainWindow() {}

void MainWindow::createMenus() {
    auto fileMenu = menuBar()->addMenu(tr("File"));

    // Crear acciones con 'this' como padre para que findChild las encuentre
    QAction *actNew = new QAction(tr("Crear"), this);
    actNew->setObjectName("actionNew");
    actNew->setShortcut(QKeySequence::New); // Ctrl+N
    actNew->setShortcutContext(Qt::ApplicationShortcut);
    connect(actNew, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(actNew);

    QAction *actOpen = new QAction(tr("Abrir archivo"), this);
    actOpen->setObjectName("actionOpen");
    actOpen->setShortcut(QKeySequence::Open); // Ctrl+O
    actOpen->setShortcutContext(Qt::ApplicationShortcut);
    connect(actOpen, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(actOpen);

    QAction *actSave = new QAction(tr("Guardar"), this);
    actSave->setObjectName("actionSave");
    actSave->setShortcut(QKeySequence::Save); // Ctrl+S
    actSave->setShortcutContext(Qt::ApplicationShortcut);
    connect(actSave, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(actSave);

    fileMenu->addSeparator();

    QAction *actQuit = new QAction(tr("Salir"), this);
    actQuit->setObjectName("actionQuit");
    actQuit->setShortcut(QKeySequence::Quit);
    actQuit->setShortcutContext(Qt::ApplicationShortcut);
    connect(actQuit, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(actQuit);

    auto buildMenu = menuBar()->addMenu(tr("Build"));

    QAction *actBuild = new QAction(tr("Compilar"), this);
    actBuild->setObjectName("actionBuild");
    actBuild->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B)); // Ctrl+B
    actBuild->setShortcutContext(Qt::ApplicationShortcut);
    connect(actBuild, &QAction::triggered, this, &MainWindow::buildProject);
    buildMenu->addAction(actBuild);

    QAction *actRun = new QAction(tr("Ejecutar"), this);
    actRun->setObjectName("actionRun");
    actRun->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R)); // Ctrl+R
    actRun->setShortcutContext(Qt::ApplicationShortcut);
    connect(actRun, &QAction::triggered, this, &MainWindow::runProject);
    buildMenu->addAction(actRun);
}

void MainWindow::createToolbar() {
    auto tb = addToolBar(tr("Main"));

    // Reutilizamos las acciones creadas en createMenus() (evita duplicados)
    QAction *actNew = findChild<QAction *>("actionNew");
    if (actNew) tb->addAction(actNew);
    else {
        QAction *fallbackNew = tb->addAction(tr("Nuevo"));
        connect(fallbackNew, &QAction::triggered, this, &MainWindow::newFile);
    }

    QAction *actOpen = findChild<QAction *>("actionOpen");
    if (actOpen) tb->addAction(actOpen);
    else {
        QAction *fallbackOpen = tb->addAction(tr("Abrir"));
        connect(fallbackOpen, &QAction::triggered, this, &MainWindow::openFile);
    }

    QAction *actSave = findChild<QAction *>("actionSave");
    if (actSave) tb->addAction(actSave);
    else {
        QAction *fallbackSave = tb->addAction(tr("Guardar"));
        connect(fallbackSave, &QAction::triggered, this, &MainWindow::saveFile);
    }

    tb->addSeparator();

    QAction *actBuild = findChild<QAction *>("actionBuild");
    if (actBuild) tb->addAction(actBuild);
    else {
        QAction *fallbackBuild = tb->addAction(tr("Compilar"));
        connect(fallbackBuild, &QAction::triggered, this, &MainWindow::buildProject);
    }

    QAction *actRun = findChild<QAction *>("actionRun");
    if (actRun) tb->addAction(actRun);
    else {
        QAction *fallbackRun = tb->addAction(tr("Ejecutar"));
        connect(fallbackRun, &QAction::triggered, this, &MainWindow::runProject);
    }
}

void MainWindow::createDocks() {
    m_fsModel = new QFileSystemModel(this);
    m_fsModel->setRootPath(QDir::currentPath());
    m_projectTree = new QTreeView(this);
    m_projectTree->setModel(m_fsModel);
    m_projectTree->setRootIndex(m_fsModel->index(QDir::currentPath()));
    auto dock = new QDockWidget(tr("Proyecto"), this);
    dock->setWidget(m_projectTree);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::applyBluePalette() {
    QPalette pal;
    pal.setColor(QPalette::Window, QColor(12, 20, 35));
    pal.setColor(QPalette::WindowText, QColor(220, 230, 245));
    pal.setColor(QPalette::Base, QColor(18, 28, 48));
    pal.setColor(QPalette::AlternateBase, QColor(22, 36, 60));
    pal.setColor(QPalette::ToolTipBase, QColor(240, 248, 255));
    pal.setColor(QPalette::ToolTipText, QColor(10, 18, 30));
    pal.setColor(QPalette::Text, QColor(220, 230, 245));
    pal.setColor(QPalette::Button, QColor(24, 40, 68));
    pal.setColor(QPalette::ButtonText, QColor(220, 230, 245));
    pal.setColor(QPalette::BrightText, QColor(255, 0, 0));
    pal.setColor(QPalette::Highlight, QColor(30, 90, 170));
    pal.setColor(QPalette::HighlightedText, QColor(250, 250, 255));
    qApp->setPalette(pal);

    setStyleSheet(
        "QMainWindow, QMenuBar, QMenu, QToolBar, QStatusBar { background-color: #0c1423; color: #dce6f5; }\n"
        "QTreeView, QTextEdit, QPlainTextEdit { background-color: #121c30; color: #dce6f5; selection-background-color: #1e5aaa; }\n"
        "QToolButton, QPushButton { background-color: #182844; color: #dce6f5; border: 1px solid #1e2f52; padding: 4px 8px; }\n"
        "QToolButton:hover, QPushButton:hover { background-color: #20375e; }\n"
        "QMenu::item:selected { background-color: #1e5aaa; }\n"
    );
}

void MainWindow::newFile() {
    m_editor->newDocument();
}

void MainWindow::openFile() {
    const QString file = QFileDialog::getOpenFileName(
        this, tr("Open File"), QDir::currentPath(),
        tr("C/C++ Files (*.h *.hpp *.c *.cpp);;All Files (*.*)"));
    if (!file.isEmpty()) {
        m_editor->openFile(file);
    }
}

void MainWindow::saveFile() {
    m_editor->save();
}

void MainWindow::buildProject() {
    if (m_buildProcess) {
        m_buildProcess->kill();
        m_buildProcess->deleteLater();
    }
    m_buildProcess = new QProcess(this);
    connect(m_buildProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::onBuildReadyRead);
    connect(m_buildProcess, &QProcess::readyReadStandardError, this, &MainWindow::onBuildReadyRead);
    connect(m_buildProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onBuildFinished);

    const QString buildDir = QDir::currentPath() + "/build";
    if (!QDir(buildDir).exists()) {
        QDir().mkpath(buildDir);
    }

    QProcess *configure = new QProcess(this);
    configure->setProgram("cmake");
    configure->setArguments(QStringList() << "-S" << "." << "-B" << buildDir);
    configure->setWorkingDirectory(QDir::currentPath());
    connect(configure, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, buildDir](int code, QProcess::ExitStatus st) {
                if (st != QProcess::NormalExit || code != 0) {
                    statusBar()->showMessage(tr("CMake configure failed"), 6000);
                    return;
                }
                m_buildProcess->setProgram("cmake");
                m_buildProcess->setArguments(QStringList() << "--build" << buildDir);
                m_buildProcess->setWorkingDirectory(QDir::currentPath());
                m_buildProcess->start();
            });
    configure->start();
}

void MainWindow::runProject() {
    QString exePath = QDir::currentPath() + "/build/Amell-IDE";
#ifdef Q_OS_WIN
    exePath += ".exe";
#endif
    if (!QFileInfo::exists(exePath)) {
        QMessageBox::warning(this, tr("Ejecutar"), tr("Ejecutable no encontrado. Compila primero."));
        return;
    }
    QProcess::startDetached(exePath);
}

void MainWindow::onBuildReadyRead() {
    const QByteArray out = m_buildProcess->readAllStandardOutput() + m_buildProcess->readAllStandardError();
    statusBar()->showMessage(QString::fromLocal8Bit(out));
}

void MainWindow::onBuildFinished(int exitCode, QProcess::ExitStatus status) {
    if (status == QProcess::NormalExit && exitCode == 0) {
        statusBar()->showMessage(tr("Compilación exitosa"), 4000);
    } else {
        statusBar()->showMessage(tr("Compilación fallida"), 6000);
    }
}
