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


#include "safevector.h"
#include "iostream"

using namespace std; 

template<class T>
SafeVector<T>::SafeVector() : vector<T> ()
{

}

template<class T>
SafeVector<T>::~SafeVector()
{

}

template<class T>
T& SafeVector<T>::operator[](int i)
{

    if (i>int(this->size())-1)
    {   cout<<int(this->size());
        cout<<"Exceeded the size"<<std::endl;
        T x;
        return x;
    }
    else if (i<0)
    {
        cout<<"Counter is negative!"<<std::endl;
        T x;
        return x;
    }
    else
        return vector<T>::operator[](i);

}

template<class T>
T& SafeVector<T>::operator[](unsigned int i)
{

    if (i > int(this->size()) - 1)
    {
        cout << int(this->size());
        cout << "Exceeded the size" << std::endl;
        T x;
        return x;
    }
    else
        return vector<T>::operator[](i);

}

template<class T>
bool SafeVector<T>::SetVal(unsigned int i, const T &val)
{
    if (i>int(this->size())-1)
    {
        cout<<"Exceeded the size"<<std::endl;
        return false;
    }
    else if (i<0)
    {
        cout<<"Counter is negative!"<<std::endl;
        return false;
    }
    vector<T>::at(i)=val;

}

template<class T>
unsigned int SafeVector<T>::lookup(const T &x)
{
    for (unsigned int i=0; i<this->size(); i++)
    {
        if (this->at(i) == x)
            return i;
    }
    return -1;
}


