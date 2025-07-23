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


#include "QuickSort.h"
#include <iostream>
#include <algorithm>	// std::swap()
#include <vector>
#include "time.h"
#ifdef QT_version
#include "qdebug.h"
#endif // QT_version
using namespace std;

CQuickSort::CQuickSort(void)
{
}


CQuickSort::~CQuickSort(void)
{

}



vector<double> QbSort(const vector<double> &V1)
{

	if (V1.size() < 100) return bubbleSort(V1);
	if (V1.size() <= 1) return V1;
	vector<double> V = V1;
	int end = V.size();
	if (V[end - 1]<V[0]) V = reverse_order(V);
	vector<double> less, greater;
	greater.push_back(V[end - 1]);
	for (int i = 0; i<end - 1; i++)
		if (V[i]<V[end - 1]) less.push_back(V[i]);
		else greater.push_back(V[i]);


		if ((V == greater) && (less.size() == 0))
			return greater;
		vector<double> res = QSort(less);
		vector<double> x2 = QSort(greater);

		res.insert(res.end(), x2.begin(), x2.end());
		less.clear();
		greater.clear();
		x2.clear();
		return res;

}


vector<int> QbSort(const vector<int> &V1)
{
	if (V1.size() < 100) return bubbleSort(V1);
	if (V1.size() <= 1) return V1;
	vector<int> V = V1;
	int end = V.size();
	if (V[end - 1]<V[0]) V = reverse_order(V);
	vector<int> less, greater;
	greater.push_back(V[end - 1]);
	for (int i = 0; i<end - 1; i++)
		if (V[i]<V[end - 1]) less.push_back(V[i]);
		else greater.push_back(V[i]);


		vector<int> res = QSort(less);
		if ((V == greater) && (less.size() == 0)) return greater;
		vector<int> x2 = QSort(greater);

		res.insert(res.end(), x2.begin(), x2.end());
		less.clear();
		greater.clear();
		x2.clear();
		return res;

}



vector<double> bubbleSort(const vector<double> &V)
{

	if (V.size() <= 1) return V;
	vector<double> A;
	if (V[V.size() - 1] < V[0])
		A = reverse_order(V);
	else
		A = V;

	int n = A.size();

	bool swapped = false;
	do
	{
		swapped = false;
		for (int i = 1; i < n - 1; i++)
		{
			if (A[i - 1] > A[i])
			{
				//temp = A[i - 1];
				//A[i - 1] = A[i];
				//A[i] = temp;
				swap(A[i], A[i - 1]);
				swapped = true;
			}
		}
	} while (swapped);
	//clock_t t1 = clock() - t0;
	//float run_time = ((float)t1) / CLOCKS_PER_SEC;
	//qDebug() << "sorting finished in" << run_time << " sec";
	return A;
}
vector<int> bubbleSort(const vector<int> &V)
{
	if (V.size() <= 1) return V;
	vector<int> A;
	if (V[V.size() - 1] < V[0])
		A = reverse_order(V);
	else
		A = V;
	int n = A.size();

	bool swapped = false;
	do
	{
		swapped = false;
		for (int i = 1; i < n - 1; i++)
		{
			if (A[i - 1] > A[i])
			{
				//temp = A[i - 1];
				//A[i - 1] = A[i];
				//A[i] = temp;
				swap(A[i], A[i - 1]);
				swapped = true;
			}
		}
	} while (!swapped);
	return A;
}




