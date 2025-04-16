#include <QCoreApplication>
#include "serverops.h"

int main(int argc, char *argv[])
{

    QCoreApplication a(argc,argv);
    ServerOps server(8080);
    a.exec();
}
