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


#include "serverops.h"
#include <QDebug>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include "System.h"
#include "Script.h"
#include <QCoreApplication>
#include <QFileInfo>
#include "weatherretriever.h"


using namespace crow;

ServerOps::ServerOps(QObject *parent) : QObject(parent)
{
    // Handle POST /calculate (main API)
    CROW_ROUTE(app, "/calculate").methods("POST"_method)
    ([this](const crow::request& req) {
        return StatementReceived(req);
    });

}

void ServerOps::Start(quint16 port)
{
    app.port(port).multithreaded().run();
}

crow::response ServerOps::StatementReceived(const crow::request& req)
{
    QByteArray jsonBytes = QByteArray::fromStdString(req.body);
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return crow::response(400, "Invalid JSON");
    }


    QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        QString key = it.key();
        QJsonValue value = it.value();
        qDebug() << "Key:" << key << ", Value:" << value;
    }

    QJsonDocument responseDoc = Execute(obj);
    QByteArray responseJson = responseDoc.toJson();

    crow::response res;
    res.code = 200;
    res.set_header("Access-Control-Allow-Origin", "*");  // or "http://localhost:8000" for more security
    res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.body = responseJson.toStdString();
    return res;
}


ServerOps::~ServerOps()
{

}


QJsonDocument ServerOps::Execute(const QJsonObject &instructions)
{
    System *system=new System();
    cout<<"Reading script ..."<<endl;
    string defaulttemppath = QCoreApplication::applicationDirPath().toStdString() + "/resources/";
    cout << "Default Template path = " + defaulttemppath +"\n";
    system->SetDefaultTemplatePath(defaulttemppath);
    system->SetWorkingFolder(QFileInfo(modelFile).canonicalPath().toStdString() + "/");

    qDebug()<<"Model File: " << modelFile;
    string settingfilename = qApp->applicationDirPath().toStdString() + "/resources/settings.json";
    Script scr(modelFile.toStdString(),system);
    cout<<"Executing script ..."<<endl;
    system->CreateFromScript(scr,settingfilename);
    system->SetSilent(false);
    for (QJsonObject::const_iterator entity = instructions.constBegin(); entity != instructions.constEnd(); ++entity)
    {
        if (system->object(entity.key().toStdString()))
        {
            QJsonObject properties = entity.value().toObject();
            qDebug()<<entity.key()<<":"<<entity.value();
            for (QJsonObject::const_iterator property = properties.constBegin(); property != properties.constEnd(); ++property)
            {
                qDebug()<<property.key()<<":"<<property.value();
                system->object(entity.key().toStdString())->SetProperty(property.key().toStdString(), property.value().toString().toStdString());
            }
        }
    }

    cout<<"Solving ..."<<endl;
    system->Solve();
    system->SavetoJson("System.json",system->addedtemplates);
    System system2;

    system2.SetDefaultTemplatePath(defaulttemppath);
    system2.SetWorkingFolder(QFileInfo(workingDirectory).canonicalPath().toStdString() + "/");
    qDebug()<<"Working Folder: "<< QString::fromStdString(system2.GetWorkingFolder());
    system2.ReadSystemSettingsTemplate(settingfilename);

    QFile file("System.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray rawData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(rawData);


    system2.LoadfromJson(doc);
    system2.SavetoScriptFile("Recreated.ohq");
    cout<<"Writing outputs in '"<< system->GetWorkingFolder() + system->OutputFileName() +"'";
    system->GetObservedOutputs().write(system->GetWorkingFolder() + system->OutputFileName());
    return QJsonDocument(system->GetObservedOutputs().toJson());

}


QJsonObject loadJsonObjectFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << filePath;
        return QJsonObject();
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return QJsonObject();
    }

    return doc.object();
}

