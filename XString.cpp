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


#include "XString.h"

XString::XString(const double &X)
{
	*this = QString::number(X);
}

XString::XString(const int &X)
{
	*this = QString::number(X);
}

XString& XString::operator+=(const XString x)
{
	*this = x.toDouble() + toDouble(); 
	return *this; 
}

XString XString::operator=(const double x)
{
	*this = QString::number(x);
	return *this;
}

XString XString::operator=(const int x)
{
	*this = QString::number(x);
	return *this; 
}
