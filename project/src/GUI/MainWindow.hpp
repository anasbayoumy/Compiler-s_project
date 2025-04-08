#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP


#include <QWidget>
#include <QApplication>


#include <QMainWindow>
#include <QTextEdit>
#include <QTableWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QMenu>
#include <QToolBar>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QScreen>
#include <QRect>
#include "lexer/Lexer.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void runLexer();
    void about();
    void updateStatusBar();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createCentralWidget();
    void readSettings();
    void writeSettings();
    void closeEvent(QCloseEvent *event) override;
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);

    // Widgets
    QTextEdit *codeEditor;
    QTableWidget *tokenViewer;
    QStatusBar *statusBar;
    QSplitter *splitter;
    QMenu *fileMenu;
    QMenu *runMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *runToolBar;

    // Actions
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *exitAct;
    QAction *runAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    // Current file
    QString currentFile;
    bool isUntitled;
}; 