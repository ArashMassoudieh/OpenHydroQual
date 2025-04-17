#include "serverops.h"
#include <QDebug>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>
#include "System.h"
#include "Script.h"
#include <QCoreApplication>
#include <QFileInfo>

using namespace crow;

ServerOps::ServerOps(quint16 port, QObject *parent): QObject(parent)
{
    CROW_ROUTE(app, "/calculate").methods("POST"_method)
    ([this](const crow::request& req) {
        return StatementReceived(req);
    });

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
    res.set_header("Content-Type", "application/json");
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
    string defaulttemppath = QCoreApplication::applicationDirPath().toStdString() + "/../../resources/";
    cout << "Default Template path = " + defaulttemppath +"\n";
    system->SetDefaultTemplatePath(defaulttemppath);
    system->SetWorkingFolder(QFileInfo(QString::fromStdString("ServerEx.ohq")).canonicalPath().toStdString() + "/");
    string settingfilename = qApp->applicationDirPath().toStdString() + "/../../resources/settings.json";
    Script scr("ServerEx.ohq",system);
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
    cout<<"Writing outputs in '"<< system->GetWorkingFolder() + system->OutputFileName() +"'";
    system->GetOutputs().writetofile(system->GetWorkingFolder() + system->OutputFileName());
    return QJsonDocument(system->GetOutputs().toJson());
}



