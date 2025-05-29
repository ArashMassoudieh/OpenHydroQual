/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


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
