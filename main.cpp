#include "mainwindow.h"
#include "request.h"
#include "answer.h"
#include "dialog.h"
#include "QDebug"
#include <QApplication>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedSize(w.size());
    w.show();

    return a.exec();
}
