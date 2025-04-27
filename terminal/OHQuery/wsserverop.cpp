#include "wsserverop.h"
#include "System.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QFileInfo>
#include "Script.h"
#include <QFile>
#include "Wizard_Script.h"
#include <QDir>

WSServerOps::WSServerOps(QObject *parent)
    : QObject(parent),
    m_server(new QWebSocketServer(QStringLiteral("Echo Server"),
                                  QWebSocketServer::NonSecureMode, this))
{


}

void WSServerOps::Start(quint16 port)
{
    if (m_server->listen(QHostAddress::Any, port)) {
        qDebug() << "WebSocket server listening on port" << port;
        connect(m_server, &QWebSocketServer::newConnection,
                this, &WSServerOps::onNewConnection);
    }
}

WSServerOps::~WSServerOps()
{
    m_server->close();
    qDeleteAll(m_clients);
}

void WSServerOps::onNewConnection()
{
    QWebSocket *socket = m_server->nextPendingConnection();

    connect(socket, &QWebSocket::textMessageReceived,
            this, &WSServerOps::onTextMessageReceived);
    connect(socket, &QWebSocket::disconnected,
            this, &WSServerOps::onSocketDisconnected);

    m_clients.append(socket);
    qDebug() << "New client connected!";


}

void WSServerOps::onTextMessageReceived(QString message)
{
    QWebSocket *senderSocket = qobject_cast<QWebSocket *>(sender());
    if (senderSocket) {
        qDebug() << "Received message from client:" << message;

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error:" << parseError.errorString();
        } else {
            qDebug() << "Parsed JSON:" << jsonDoc;
        }

        QJsonObject root_obj = jsonDoc.object();
        if (root_obj.keys().contains("Model"))
        {
            QJsonDocument responseDoc = SendModelTemplate(root_obj["Model"].toObject()["FileName"].toString());
            qDebug()<<responseDoc;
            QString jsonString = QString::fromUtf8(responseDoc.toJson(QJsonDocument::Compact));
            sendMessageToClient(senderSocket, jsonString);
        }
        else if (root_obj.keys().contains("Parameters"))
        {
            QJsonObject obj = root_obj["Parameters"].toObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                QString key = it.key();
                QJsonValue value = it.value();
                qDebug() << "Key:" << key << ", Value:" << value;
            }
            WizardScript SelectedWizardScript(TemplateFile_Fullpath);
            SelectedWizardScript.AssignParameterValues(obj);

            QString randomFolderName = QUuid::createUuid().toString(QUuid::WithoutBraces);

            // Set the path where you want to create the folder
            QString basePath = QDir::homePath();
            QString newFolderPath = basePath + "/OHQueryTemporaryFolder/" + randomFolderName;

            // Create the directory
            QDir dir;

            QString MainFolderPath = basePath + "/OHQueryTemporaryFolder/";

            if (!dir.exists(MainFolderPath)) {
                if (dir.mkpath(MainFolderPath)) {
                    qDebug() << "Folder created successfully:" << MainFolderPath;
                } else {
                    qDebug() << "Failed to create folder:" << MainFolderPath;
                }
            } else {
                qDebug() << "Folder already exists:" << MainFolderPath;
            }

            if (dir.mkpath(newFolderPath)) {
                qDebug() << "Folder created successfully:" << newFolderPath;
            } else {
                qDebug() << "Failed to create folder.";
            }

            SelectedWizardScript.SetWorkingFolder(newFolderPath);

            Script script;
            System system;
            string defaulttemppath = QCoreApplication::applicationDirPath().toStdString() + "/../../resources/";
            system.ReadSystemSettingsTemplate(qApp->applicationDirPath().toStdString() + "/../../resources/settings.json");
            cout << "Default Template path = " + defaulttemppath +"\n";
            system.SetDefaultTemplatePath(defaulttemppath);
            system.SetWorkingFolder(newFolderPath.toStdString());
            script.CreateSystemFromQStringList(SelectedWizardScript.Script(),&system);
            QJsonDocument responseDoc = Execute(&system);
            QString jsonString = QString::fromUtf8(responseDoc.toJson(QJsonDocument::Compact));
            sendMessageToClient(senderSocket, jsonString);
        }
    }
}

QJsonDocument WSServerOps::SendModelTemplate(const QString &TemplateName)
{
    QFile file(QCoreApplication::applicationDirPath() + "/../../resources/Wizard_Scripts_server/" + TemplateName);
    qDebug()<<QCoreApplication::applicationDirPath() + "/../../resources/Wizard_Scripts_server/" + TemplateName;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file:" << QCoreApplication::applicationDirPath() + "/../../resources/Wizard_Scripts_server/" + TemplateName;
        return QJsonDocument(); // returns a null document
    }
    else
    {
        TemplateFile_Fullpath = QCoreApplication::applicationDirPath() + "/../../resources/Wizard_Scripts_server/" + TemplateName;
    }
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return QJsonDocument();
    }

    return jsonDoc;
}

void WSServerOps::onSocketDisconnected()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket) {
        m_clients.removeAll(socket);
        socket->deleteLater();
        qDebug() << "Client disconnected!";
    }
}


void WSServerOps::sendMessageToClient(QWebSocket *client, const QString &message)
{
    if (client) {
        client->sendTextMessage(message);
        //qDebug() << "Sent message to client:" << message;
    }
}

QJsonDocument WSServerOps::Execute(System *system)
{

    string settingfilename = qApp->applicationDirPath().toStdString() + "/../../resources/settings.json";

    cout<<"Executing script ..."<<endl;

    system->SetSilent(false);
    system->SavetoScriptFile(system->GetWorkingFolder() + "/" + "System.ohq");
    cout<<"Solving ..."<<endl;
    system->Solve();
    system->SavetoJson(system->GetWorkingFolder() + "/" + "System.json",system->addedtemplates);


    cout<<"Writing outputs in '"<< system->GetWorkingFolder() + "/" + system->OutputFileName() +"'";
    system->GetObservedOutputs().writetofile(system->GetWorkingFolder() + "/" + system->ObservedOutputFileName());
    system->GetOutputs().writetofile(system->GetWorkingFolder() + "/" + system->OutputFileName());
    return QJsonDocument(system->GetObservedOutputs().toJson());
}




