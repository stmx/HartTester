#include "mainwindow.h"
#include "request.h"
#include "answer.h"
#include "QDebug"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setFixedSize(w.size());
    return a.exec();
}
