#ifndef WIZARD_ASSIGNED_VALUE_H
#define WIZARD_ASSIGNED_VALUE_H

#include <QString>

class Wizard_Assigned_Value
{
public:
    Wizard_Assigned_Value();
    Wizard_Assigned_Value(const Wizard_Assigned_Value& assigned_value);
    Wizard_Assigned_Value& operator=(const Wizard_Assigned_Value& assigned_value);
    void SetAsParam(bool b) {isparameter = b;}
    void SetParam(const QString &param) {parameter = param;}
    void SetValue(QString _value) {value = _value;}
private:
    bool isparameter = false;
    QString parameter;
    QString value;
};

#endif // WIZARD_ASSIGNED_VALUE_H



