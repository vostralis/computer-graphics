#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QPalette bgc;
    bgc.setColor(QPalette::Window, QColor(210, 210, 210));
    w.setPalette(bgc);

    w.show();
    return a.exec();
}
