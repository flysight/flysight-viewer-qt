#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // Import a file if specified on the command line
    if (QCoreApplication::arguments().size() >= 2)
    {
        w.importFile(QCoreApplication::arguments().at(1));
    }
    
    return a.exec();
}
