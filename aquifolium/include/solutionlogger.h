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


#ifndef SOLUTIONLOGGER_H
#define SOLUTIONLOGGER_H
#include <fstream>
#include "Vector.h"
#include "Matrix.h"
#include "Matrix_arma.h"
#include "Vector_arma.h"

using namespace std;

class SolutionLogger
{
public:
    SolutionLogger();
    SolutionLogger(const string &filename);
    bool AssignFile(const string &filename);
    bool WriteString(const string &s);
    bool Close();
    bool WriteVector(const CVector &v);
    bool WriteVector(const CVector_arma &v);
    bool WriteMatrix(const CMatrix &v);
    bool WriteMatrix(const CMatrix_arma &v);
    void Flush();
private:
    ofstream file;
    vector<string> contents;
};

#endif // SOLUTIONLOGGER_H
