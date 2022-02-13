#include "mainwindow.h"
#include <QApplication>

//#undef ARMA_USE_OPENMP

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifndef mac_version
    a.setWindowIcon(QIcon(qApp->applicationDirPath() + "/../../resources/icons/Aquifolium.png"));
#else
    a.setWindowIcon(QIcon(qApp->applicationDirPath() + "/../resources/icons/Aquifolium.png"));

#endif
    MainWindow w;
    w.show();

    return a.exec();
}
