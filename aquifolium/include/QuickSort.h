#pragma once
#include <vector>

using namespace std;
class CQuickSort
{
public:
	CQuickSort(void);
	~CQuickSort(void);
};

vector<double> QSort(const vector<double> &V);
vector<double> QbSort(const vector<double> &V);
vector<int> QSort(const vector<int> &V);
vector<int> QbSort(const vector<int> &V);
vector<double> reverse_order(const vector<double> &V);
vector<int> reverse_order(const vector<int> &V);
vector<double> bubbleSort(const vector<double> &V);
vector<int> bubbleSort(const vector<int> &V);
