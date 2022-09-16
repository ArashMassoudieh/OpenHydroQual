#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include "wizard_assigned_value.h"

class MajorBlock
{
public:
	MajorBlock();
    MajorBlock(const QJsonObject& jsonobject);
	MajorBlock(const MajorBlock& WS);
	MajorBlock& operator=(const MajorBlock& WS);
	QString Name();
    QString Type();
    QString V_ConnectorType();
    QString H_ConnectorType();
private:
    QMap<QString, Wizard_Assigned_Value> Arguments;
    QString name;
    QString type;
    QString v_connector_type;
    QString h_connector_type;

};
