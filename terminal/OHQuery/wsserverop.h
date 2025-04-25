#ifndef WSSERVEROP_H
#define WSSERVEROP_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>


class WSServerOps : public QObject
{
    Q_OBJECT
public:
    explicit WSServerOps(QObject *parent = nullptr);
    void Start(quint16 port);
    ~WSServerOps();
    QJsonDocument Execute(const QJsonObject &instructions);
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
    QJsonDocument SendModelTemplate(const QString &TemplateName);
private slots:
    void onNewConnection();
    void onTextMessageReceived(QString message);
    void onSocketDisconnected();
    void sendMessageToClient(QWebSocket *client, const QString &message);


private:
    QWebSocketServer *m_server;
    QList<QWebSocket *> m_clients;
    QString modelFile;
    QString workingDirectory;
    QString TemplateFile_Fullpath;

};

#endif // WEBSOCKETSERVER_H


