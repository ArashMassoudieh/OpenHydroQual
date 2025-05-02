#pragma once
#include <qstring.h>
#include <qjsonobject.h>
#include <QVector> 
#include "Wizard_Entity.h"

class SingleBlock: public Wizard_Entity
{
public:
    SingleBlock();
    SingleBlock(const QJsonObject& jsonobject);
    SingleBlock(const SingleBlock& WS);
    SingleBlock& operator=(const SingleBlock& WS);
    QStringList GenerateScript(QMap<QString, WizardParameter>* params);
private:



};
