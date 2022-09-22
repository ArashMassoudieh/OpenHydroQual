#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "Wizard_Argument.h"
#include <QVector> 

class MajorBlock
{
public:
	MajorBlock();
    MajorBlock(const QJsonObject& jsonobject);
	MajorBlock(const MajorBlock& WS);
	MajorBlock& operator=(const MajorBlock& WS);
	QString Name();
    QString Type();
    QString GridType();
    QString V_ConnectorType();
    QString H_ConnectorType();
    QStringList GenerateScript(QMap<QString, WizardParameter>* params);
private:
    QString gridtype; 
    QMap<QString, Wizard_Argument> Arguments;
    QString name;
    QString type;
    QString v_connector_type;
    QString h_connector_type;

};
