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


#ifndef CONSTITUENT_H
#define CONSTITUENT_H

#include "Expression.h"
#include "BTC.h"
#include "Object.h"

class System;

class Constituent: public Object
{
public:
    Constituent();
    virtual ~Constituent();
    Constituent(const Constituent& other);
    Constituent& operator=(const Constituent& other);
    bool SetName(const string &s, bool setprop=true);

};

#endif // CONSTITUENT_H
