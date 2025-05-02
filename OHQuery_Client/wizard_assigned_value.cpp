#include "wizard_assigned_value.h"

Wizard_Assigned_Value::Wizard_Assigned_Value()
{

}



Wizard_Assigned_Value::Wizard_Assigned_Value(const Wizard_Assigned_Value& assigned_value)
{
    value = assigned_value.value;
    parameter = assigned_value.parameter;
    isparameter = assigned_value.isparameter;
}
Wizard_Assigned_Value& Wizard_Assigned_Value::operator=(const Wizard_Assigned_Value& assigned_value)
{
    value = assigned_value.value;
    parameter = assigned_value.parameter;
    isparameter = assigned_value.isparameter;
    return *this;
}
