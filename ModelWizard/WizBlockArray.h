#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "Wizard_Argument.h"
#include <QVector> 
#include "Wizard_Entity.h"

class BlockArray: public Wizard_Entity
{
public:
    BlockArray();
    BlockArray(const QJsonObject& jsonobject);
    BlockArray(const BlockArray& WS);
    BlockArray& operator=(const BlockArray& WS);
    QString GridType();
    QString V_ConnectorType();
    QString H_ConnectorType();
    QStringList GenerateScript(QMap<QString, WizardParameter>* params);
    int Nx() { return nx; }
    int Ny() { return ny; }
    QString BlockName(int i, int j);
private:
    QString gridtype; 
    QMap<QString, Wizard_Argument> Arguments_H;
    QMap<QString, Wizard_Argument> Arguments_V;
    QString v_connector_type;
    QString h_connector_type;
    int nx = 0; 
    int ny = 0; 

};
