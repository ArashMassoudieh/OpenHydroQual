/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


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
