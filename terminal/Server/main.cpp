#include <QCoreApplication>
#include "serverops.h"
#include "wsserverop.h"

int main(int argc, char *argv[])
{

    QCoreApplication a(argc,argv);
    QJsonObject configuration = loadJsonObjectFromFile("config.json");
    ServerOps *server = nullptr;
    WSServerOps *wsserver = nullptr;
    if (configuration["Config"].toString() == "FlaskType")
    {   server = new ServerOps();
        server->SetModelFile(configuration["ModelFile"].toString());
        server->SetWorkingDirectory(configuration["WorkingDirectory"].toString());
        server->Start(8080);
    }
    else if (configuration["Config"].toString() == "WebSockets")
    {
        wsserver = new WSServerOps();
        wsserver->SetModelFile(configuration["ModelFile"].toString());
        wsserver->SetWorkingDirectory(configuration["WorkingDirectory"].toString());
        wsserver->Start(12345);
    }
    a.exec();
}
