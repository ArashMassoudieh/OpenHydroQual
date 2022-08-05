#ifndef SAFEVECTOR_H
#define SAFEVECTOR_H

#include <vector>

using namespace std;

template<class T>
class SafeVector: public vector<T>
{
public:
    SafeVector();
    ~SafeVector();
    T& operator[](int i);
    T& operator[](unsigned int i);
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
    bool SetVal(unsigned int i, const T &val);
    unsigned int lookup(const T &x);
    void append(const SafeVector<T> &v)
    {
        this->insert(this->end(), v.begin(), v.end());
    }
    void append(const T &x)
    {
        this->push_back(x);
    }

};


#include "safevector.hpp"

#endif // SAFEVECTOR_H
