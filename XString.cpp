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
