#include <QCoreApplication>
#include "serverops.h"
#include "wsserverop.h"

int main(int argc, char *argv[])
{

    QCoreApplication a(argc,argv);
    QJsonObject configuration = loadJsonObjectFromFile("config.json");
    ServerOps *server = nullptr;


#ifdef HTTPS
    int port_number = 12345;
#else
    int port_number = 12345;
#endif

#ifndef LOCAL_HOST
    QString certPath = "/etc/letsencrypt/live/greeninfraiq.com/fullchain.pem";
    QString keyPath = "/etc/letsencrypt/live/greeninfraiq.com/privkey.pem";
#else
    QString certPath = QStringLiteral("/home/arash/localhost-ssl/fullchain.pem");
    QString keyPath = QStringLiteral("/home/arash/localhost-ssl/privkey.pem");
#endif
    qDebug()<<"Certificate Path: "<<certPath;
    qDebug()<<"Key Path: "<<keyPath;
    WSServerOps *wsserver = nullptr;
    if (configuration["Config"].toString() == "FlaskType")
    {   server = new ServerOps();
        server->SetModelFile(configuration["ModelFile"].toString());
        server->SetWorkingDirectory(configuration["WorkingDirectory"].toString());
        server->Start(8080);
    }
    else if (configuration["Config"].toString() == "WebSockets")
    {
#ifdef HTTPS
        wsserver = new WSServerOps(certPath, keyPath);
#else
        wsserver = new WSServerOps();
#endif
        wsserver->SetModelFile(configuration["ModelFile"].toString());
        wsserver->SetWorkingDirectory(configuration["WorkingDirectory"].toString());
        wsserver->Start(port_number);
    }
    a.exec();
}
