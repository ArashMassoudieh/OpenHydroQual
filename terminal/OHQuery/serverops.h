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


#ifndef SERVEROPS_H
#define SERVEROPS_H

#include <QObject>

#ifdef signals
#undef signals
#endif
#include "crow.h"
#include <QJsonObject>
#include "cors_middleware.h"
#include <QFile>
#include <QJsonParseError>

class ServerOps : public QObject
{
    Q_OBJECT
public:
    explicit ServerOps(QObject *parent = nullptr);
    void Start(quint16 port);
    ~ServerOps();
    crow::response StatementReceived(const crow::request& req);
    QJsonDocument Execute(const QJsonObject &instructions = QJsonObject());
    void SetModelFile(const QString &modelfile)
    {
        qDebug()<< "Model file was set to '" <<modelfile<<"'";
        modelFile = modelfile;
    }

    void SetWorkingDirectory(const QString &workingdirectory)
    {
        qDebug()<< "Working directory was set to '" <<workingdirectory<<"'";
        workingDirectory = workingdirectory;
    }

private slots:




private:
    crow::App<CORS> app;
    QString modelFile;
    QString workingDirectory;
};

QJsonObject loadJsonObjectFromFile(const QString& filePath);
#endif // WEBSOCKETSERVER_H
