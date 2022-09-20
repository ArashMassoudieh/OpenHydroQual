#ifndef WIZARDPARAMETER_H
#define WIZARDPARAMETER_H

#include <qstring.h>
#include <qjsonobject.h>
#include <QVector2D>

class WizardParameter
{
public:
    WizardParameter();
    WizardParameter(const QJsonObject& jsonobject);
    WizardParameter(const WizardParameter& WS);
    WizardParameter& operator=(const WizardParameter& WS);
    QString Name();
    QString Delegate();
    QString Question();
    QVector2D Range();
    QStringList Units();
    QStringList ComboItems();
    void SetEntryItem(QWidget *item) {entryitem = item;}
    QWidget *EntryItem() {return entryitem;}
    void SetValue(const QString &val) {value = val;}
    QString Value() {return value; }
private:
    QString name;
    QString delegate;
    QString question;
    QVector2D range;
    QStringList units;
    QStringList comboitems;
    QWidget *entryitem = nullptr;
    QString value;
};


#endif // WIZARDPARAMETER_H
