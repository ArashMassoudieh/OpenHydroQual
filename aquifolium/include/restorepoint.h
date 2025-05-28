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


#ifndef RESTOREPOINT_H
#define RESTOREPOINT_H

#include "System.h"

class RestorePoint
{
public:
    RestorePoint();
    RestorePoint(System *sys);
    virtual ~RestorePoint()
    {
        CopiedSystem.clear();
    }
    System *GetSystem();
    double t;
    double dt;
    unsigned int used_counter=0;
private:
    System CopiedSystem;

};

#endif // RESTOREPOINT_H
