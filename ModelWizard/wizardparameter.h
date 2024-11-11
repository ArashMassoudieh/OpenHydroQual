#ifndef WIZARDPARAMETER_H
#define WIZARDPARAMETER_H

#include <qstring.h>
#include <qjsonobject.h>
#include <QVector2D>
#include <QStringList>

enum class parameter_type {numeric, string, date};

class WizardParameter
{
public:
    WizardParameter();
    WizardParameter(const QJsonObject& jsonobject);
    WizardParameter(const WizardParameter& WS);
    WizardParameter& operator=(const WizardParameter& WS);
    QString Name();
    void SetName(const QString &_name);
    QString Delegate();
    QString Question();
    QString Default();
    QVector2D Range();
    QStringList Units();
    QStringList ComboItems();
    void SetEntryItem(QWidget *item) {entryitem = item;}
    QWidget *EntryItem() {return entryitem;}
    void SetValue(const QString &val) {value = val;}
    QString Value() {return value; }
    parameter_type ParameterType() { return Parameter_Type; }
private:
    QString name;
    QString delegate;
    QString question;
    QString default_value;
    QVector2D range;
    QStringList units;
    QStringList comboitems;
    QWidget *entryitem = nullptr;
    QString value;
    parameter_type Parameter_Type = parameter_type::numeric; 
};


#endif // WIZARDPARAMETER_H
