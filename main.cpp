#include "mainwindow.h"
#include <QApplication>

#undef ARMA_USE_OPENMP

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
