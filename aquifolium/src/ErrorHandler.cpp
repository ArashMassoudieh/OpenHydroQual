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


#include "ErrorHandler.h"

ErrorHandler::ErrorHandler()
{
    //ctor
}

ErrorHandler::~ErrorHandler()
{
    //dtor
}

ErrorHandler::ErrorHandler(const ErrorHandler& other)
{
    //copy ctor
}

ErrorHandler& ErrorHandler::operator=(const ErrorHandler& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

void ErrorHandler::Write(const string &filename)
{
    ofstream file(filename);
    file << "class, object name, function, description, error code"<<endl;
    for (unsigned int i=0; i<errors.size(); i++)
    {
        file<<errors[i].cls<<","<<errors[i].objectname<<","<<errors[i].funct<<","<<errors[i].description<<","<<errors[i].code<<endl;
    }
    file.close();
}
