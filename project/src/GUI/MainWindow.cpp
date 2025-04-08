#include "MainWindow.hpp"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QHeaderView>
#include <QIcon>
#include <QKeySequence>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextLayout>
#include <QTextOption>
#include <QTextDocumentFragment>
#include <QTextDocumentWriter>
#include <QTextCodec>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextLayout>
#include <QTextOption>
#include <QTextDocumentFragment>
#include <QTextDocumentWriter>
#include <QTextCodec>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextLayout>
#include <QTextOption>
#include <QTextDocumentFragment>
#include <QTextDocumentWriter>
#include <QTextCodec>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isUntitled(true) {
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createCentralWidget();
    readSettings();

    setWindowTitle(tr("Compiler GUI"));
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow() {
}

void MainWindow::createActions() {
    // File menu actions
    newAct = new QAction(tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveFileAs);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &QWidget::close);

    // Run action
    runAct = new QAction(tr("&Run Lexer"), this);
    runAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    runAct->setStatusTip(tr("Run the lexer on the current code"));
    connect(runAct, &QAction::triggered, this, &MainWindow::runLexer);

    // Help menu actions
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    runMenu = menuBar()->addMenu(tr("&Run"));
    runMenu->addAction(runAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars() {
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newAct);
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    runToolBar = addToolBar(tr("Run"));
    runToolBar->addAction(runAct);
}

void MainWindow::createStatusBar() {
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    connect(codeEditor->document(), &QTextDocument::contentsChanged,
            this, &MainWindow::updateStatusBar);
    updateStatusBar();
}

void MainWindow::createCentralWidget() {
    splitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(splitter);

    // Create code editor
    codeEditor = new QTextEdit(splitter);
    codeEditor->setLineWrapMode(QTextEdit::NoWrap);
    codeEditor->setAcceptRichText(false);
    codeEditor->setFont(QFont("Courier New", 10));

    // Create token viewer
    tokenViewer = new QTableWidget(splitter);
    tokenViewer->setColumnCount(4);
    tokenViewer->setHorizontalHeaderLabels({"Type", "Lexeme", "Line", "Column"});
    tokenViewer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    splitter->addWidget(codeEditor);
    splitter->addWidget(tokenViewer);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
}

void MainWindow::readSettings() {
    QSettings settings;
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings() {
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::maybeSave() {
    if (!codeEditor->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                              tr("The document has been modified.\n"
                                 "Do you want to save your changes?"),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::newFile() {
    if (maybeSave()) {
        codeEditor->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::openFile() {
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool MainWindow::saveFile() {
    if (isUntitled) {
        return saveFileAs();
    } else {
        return saveFile(currentFile);
    }
}

bool MainWindow::saveFileAs() {
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::loadFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                            tr("Cannot read file %1:\n%2.")
                            .arg(QDir::toNativeSeparators(fileName),
                                 file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    codeEditor->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                            tr("Cannot write file %1:\n%2.")
                            .arg(QDir::toNativeSeparators(fileName),
                                 file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << codeEditor->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName) {
    currentFile = fileName;
    codeEditor->document()->setModified(false);
    setWindowModified(false);

    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName) {
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::updateStatusBar() {
    QTextCursor cursor = codeEditor->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber() + 1;
    statusBar->showMessage(tr("Line: %1, Column: %2").arg(line).arg(column));
}

void MainWindow::runLexer() {
    QString code = codeEditor->toPlainText();
    std::string sourceCode = code.toStdString();
    
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    // Clear the token viewer
    tokenViewer->setRowCount(0);

    // Add tokens to the viewer
    for (const auto& token : tokens) {
        int row = tokenViewer->rowCount();
        tokenViewer->insertRow(row);

        // Convert token type to string
        QString typeStr;
        switch (token.getType()) {
            case TokenType::KEYWORD: typeStr = "KEYWORD"; break;
            case TokenType::IDENTIFIER: typeStr = "IDENTIFIER"; break;
            case TokenType::NUMBER: typeStr = "NUMBER"; break;
            case TokenType::STRING: typeStr = "STRING"; break;
            case TokenType::OPERATOR: typeStr = "OPERATOR"; break;
            case TokenType::INDENT: typeStr = "INDENT"; break;
            case TokenType::DEDENT: typeStr = "DEDENT"; break;
            case TokenType::NEWLINE: typeStr = "NEWLINE"; break;
            case TokenType::COMMENT: typeStr = "COMMENT"; break;
            case TokenType::ERROR: typeStr = "ERROR"; break;
        }

        tokenViewer->setItem(row, 0, new QTableWidgetItem(typeStr));
        tokenViewer->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(token.getLexeme())));
        tokenViewer->setItem(row, 2, new QTableWidgetItem(QString::number(token.getLine())));
        tokenViewer->setItem(row, 3, new QTableWidgetItem(QString::number(token.getColumn())));
    }

    statusBar->showMessage(tr("Lexer completed. Found %1 tokens.").arg(tokens.size()), 2000);
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About Compiler GUI"),
            tr("The <b>Compiler GUI</b> is a simple interface for testing the compiler's lexer. "
               "It allows you to write code and see the tokens generated by the lexer."));
} 