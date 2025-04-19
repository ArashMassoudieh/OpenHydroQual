#include <QCoreApplication>
#include "serverops.h"

int main(int argc, char *argv[])
{

    QCoreApplication a(argc,argv);
    ServerOps server;
    QJsonObject configuration = loadJsonObjectFromFile("config.json");
    qDebug()<<configuration["ModelFile"].toString();
    server.SetModelFile(configuration["ModelFile"].toString());
    server.SetWorkingDirectory(configuration["WorkingDirectory"].toString());
    server.Start(8080);
    a.exec();
}
