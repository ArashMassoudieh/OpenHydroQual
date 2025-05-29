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



#pragma once
#include <vector>

using namespace std;

class CBinary  
{
public:
	CBinary();
	CBinary(int n);
    CBinary(int n, int preci);
	virtual ~CBinary();
    int nDigits = 4;
    vector<int> Digit;
	int precision;
	CBinary(const CBinary &B);
    CBinary operator = (const CBinary &B);
    CBinary operator + (const CBinary &B);
    CBinary extract(int spoint, int epoint);
    double decode(double minrange);
    int& operator[](unsigned int i);
    void show();
	bool sign = true;
    void mutate(double mu);
	
};

CBinary code(double x, double minrange, double maxrange, int precision);
void cross(CBinary &B1, CBinary &B2, int p);
void cross(CBinary &B1, CBinary &B2, vector<int> p);
void cross2p(CBinary &B1, CBinary &B2, int p1, int p2);

