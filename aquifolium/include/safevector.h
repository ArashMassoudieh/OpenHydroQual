#ifndef SAFEVECTOR_H
#define SAFEVECTOR_H

#include <vector>

using namespace std;

template<class T>
class SafeVector: public vector<T>
{
public:
    SafeVector();
    T& operator[](int i);
    vector<T> toStdVector()
    {
        vector<T> out = this->vector;
    }
    static SafeVector fromStdVector(const vector<T> &x)
    {
        SafeVector s;
        s.assign(x.begin(),x.end());
        return s;
    }
    unsigned int lookup(const T &x);
    void append(const SafeVector<T> &v)
    {
        this->push_back(v);
    }

};


#include "safevector.hpp"

#endif // SAFEVECTOR_H
