#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "Wizard_Argument.h"
#include <QVector> 
#include "Wizard_Entity.h"

class MajorBlock: public Wizard_Entity
{
public:
	MajorBlock();
    MajorBlock(const QJsonObject& jsonobject);
	MajorBlock(const MajorBlock& WS);
	MajorBlock& operator=(const MajorBlock& WS);
    QString GridType();
    QString V_ConnectorType();
    QString H_ConnectorType();
    QStringList GenerateScript(QMap<QString, WizardParameter>* params);
private:
    QString gridtype; 
    QMap<QString, Wizard_Argument> Arguments_H;
    QMap<QString, Wizard_Argument> Arguments_V;
    QString v_connector_type;
    QString h_connector_type;

};
