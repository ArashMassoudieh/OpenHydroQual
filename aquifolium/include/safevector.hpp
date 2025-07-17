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
#include <iostream>

template<class T>
SafeVector<T>::SafeVector() : std::vector<T>() {}

template<class T>
SafeVector<T>::~SafeVector() {}

template<class T>
T& SafeVector<T>::operator[](int i)
{
    if (i < 0 || i >= static_cast<int>(this->size())) {
        static T dummy{};
        std::cerr << "SafeVector: Index " << i << " out of bounds [0, " << this->size() - 1 << "]\n";
        return dummy;
    }
    return std::vector<T>::operator[](i);
}

template<class T>
T& SafeVector<T>::operator[](unsigned int i)
{
    if (i >= this->size()) {
        static T dummy{};
        std::cerr << "SafeVector: Index " << i << " out of bounds [0, " << this->size() - 1 << "]\n";
        return dummy;
    }
    return std::vector<T>::operator[](i);
}

template<class T>
std::vector<T> SafeVector<T>::toStdVector() const
{
    return *this;
}

template<class T>
SafeVector<T> SafeVector<T>::fromStdVector(const std::vector<T>& x)
{
    SafeVector<T> s;
    s.assign(x.begin(), x.end());
    return s;
}

template<class T>
bool SafeVector<T>::SetVal(unsigned int i, const T& val)
{
    if (i >= this->size()) {
        std::cerr << "SafeVector: SetVal index " << i << " out of bounds\n";
        return false;
    }
    this->at(i) = val;
    return true;
}

template<class T>
unsigned int SafeVector<T>::lookup(const T& x) const
{
    for (unsigned int i = 0; i < this->size(); ++i) {
        if (this->at(i) == x)
            return i;
    }
    return this->size();  // not found
}

template<class T>
void SafeVector<T>::append(const SafeVector<T>& v)
{
    this->insert(this->end(), v.begin(), v.end());
}

template<class T>
void SafeVector<T>::append(const T& x)
{
    this->push_back(x);
}
