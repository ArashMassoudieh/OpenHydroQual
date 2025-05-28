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


#ifndef SOURCE_H
#define SOURCE_H

#include "Object.h"
#include "Quan.h"

using namespace std;

class System;
class Source: public Object
{
    public:
        Source();
        virtual ~Source();
        Source(System *_parent);
        Source(const Source& other);
        Source& operator=(const Source& rhs);
        double GetValue(Object *obj);
        void SetCorrespondingConstituent(const string &crspndngconsttnt) {corresponding_constituent = crspndngconsttnt;}
        string GetCorrespondingConstituent() {return corresponding_constituent;}
    protected:

    private:
        string corresponding_constituent = "";
};

#endif // SOURCE_H
