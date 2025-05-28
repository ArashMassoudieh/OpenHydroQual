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
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#ifndef RXNPARAMETER_H
#define RXNPARAMETER_H

#include <Object.h>

class RxnParameter : public Object
{
public:
    RxnParameter();
    RxnParameter(System *parent);
    RxnParameter(const RxnParameter& other);
    RxnParameter& operator=(const RxnParameter& rhs);
    virtual ~RxnParameter();
    bool SetName(const string &newname, bool setprop=true);
};

#endif // RXNPARAMETER_H


#include "Object.h"
#include "Expression.h"
#include <map>

