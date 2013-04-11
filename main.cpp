#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("atomic.batcave.net");
    QCoreApplication::setApplicationName("LessEdit");
    tdMainWindow w;
    QIcon icon(":/icon.png");
    w.setWindowIcon(icon);
    w.show();
    return a.exec();
}
