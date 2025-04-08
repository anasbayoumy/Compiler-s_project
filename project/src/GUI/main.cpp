#include "MainWindow.hpp"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application information
    QApplication::setApplicationName("Compiler GUI");
    QApplication::setOrganizationName("Compiler Project");
    QApplication::setApplicationVersion("1.0");
    
    // Set the style to Fusion for a modern look
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
} 