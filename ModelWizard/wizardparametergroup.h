#ifndef WIZARDPARAMETERGROUP_H
#define WIZARDPARAMETERGROUP_H

#include <QStringList>
#include <QJsonObject>
#include "wizardcriteria.h"

class WizardParameterGroup
{
public:
    WizardParameterGroup();
    WizardParameterGroup(const QJsonObject& jsonobj);
    WizardParameterGroup(const WizardParameterGroup &WPG);
    WizardParameterGroup& operator=(const WizardParameterGroup& WPG);
    QString& Parameter(int i){
        if (i<parameters.size())
            return parameters[i];
    }
    WizardCriteria& Criteria(int i){
        if (i<criteria.size())
            return criteria[i];
    }
    QString Name() {return name;}
    QString Description() {return description;}
    int ParametersCount() {return parameters.count();}
    int CriteriaCount() {return criteria.count();}

private:
    QStringList parameters;
    QList<WizardCriteria> criteria;
    QString name;
    QString description;
};

#endif // WIZARDPARAMETERGROUP_H
