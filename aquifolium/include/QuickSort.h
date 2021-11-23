#pragma once
#include <vector>

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
vector<T> QSort(const vector<T> &V1)
{

    if (V1.size() <= 1) return V1;
    vector<T> V = V1;
    int end = V.size();
    if (V[end - 1]<V[0]) V = reverse_order(V);
    vector<T> less, greater;
    greater.push_back(V[end - 1]);
    for (int i = 0; i<end - 1; i++)
        if (V[i]<V[end - 1]) less.push_back(V[i]);
        else greater.push_back(V[i]);


        if ((V == greater) && (less.size() == 0))
            return greater;
        vector<T> res = QSort(less);
        vector<T> x2 = QSort(greater);

        res.insert(res.end(), x2.begin(), x2.end());
        less.clear();
        greater.clear();
        x2.clear();
        return res;

}
