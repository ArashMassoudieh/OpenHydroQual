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
#include "Vector.h"

using namespace std;
class CQuickSort
{
public:
	CQuickSort(void);
	~CQuickSort(void);
};

template <class T>
vector<T> QSort(const vector<T> &V);
vector<double> QbSort(const vector<double> &V);
vector<int> QbSort(const vector<int> &V);

template <class T>
vector<T> reverse_order(const vector<T> &V)
{
    vector<T> A;
    for (int i = V.size() - 1; i >= 0; i--)
        A.push_back(V[i]);

    return A;
}

vector<double> bubbleSort(const vector<double> &V);
vector<int> bubbleSort(const vector<int> &V);

template <class T>
std::vector<T> QSort(const std::vector<T>& V) {
    std::vector<T> sorted = V;
    std::sort(sorted.begin(), sorted.end());
    return sorted;
}
