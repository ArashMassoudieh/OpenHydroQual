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
