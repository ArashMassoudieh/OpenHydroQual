#include <QCoreApplication>
#include "serverops.h"
#include "wsserverop.h"

int main(int argc, char *argv[])
{

    QCoreApplication a(argc,argv);
    QJsonObject configuration = loadJsonObjectFromFile("config.json");
    ServerOps *server = nullptr;

#ifndef LOCAL_HOST
    QString certPath = QStringLiteral("/etc/letsencrypt/live/yourdomain.com/fullchain.pem");
    QString keyPath = QStringLiteral("/etc/letsencrypt/live/yourdomain.com/privkey.pem");
#else
    QString certPath = QStringLiteral("/home/arash/localhost-ssl/fullchain.pem");
    QString keyPath = QStringLiteral("/home/arash/localhost-ssl/privkey.pem");
#endif

    WSServerOps *wsserver = nullptr;
    if (configuration["Config"].toString() == "FlaskType")
    {   server = new ServerOps();
        server->SetModelFile(configuration["ModelFile"].toString());
        server->SetWorkingDirectory(configuration["WorkingDirectory"].toString());
        server->Start(8080);
    }
    else if (configuration["Config"].toString() == "WebSockets")
    {
        wsserver = new WSServerOps(certPath, keyPath);
        wsserver->SetModelFile(configuration["ModelFile"].toString());
        wsserver->SetWorkingDirectory(configuration["WorkingDirectory"].toString());
        wsserver->Start(12345);
    }
    a.exec();
}
