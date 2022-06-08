#ifndef SAFEVECTOR_H
#define SAFEVECTOR_H

#include <vector>
#include <omp.h>

using namespace std;

template<class T>
class SafeVector: public vector<T>
{
public:
    SafeVector();
    ~SafeVector();
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
        this->insert(this->end(), v.begin(), v.end());
    }
    bool SetVal(unsigned int i, const T &val);
    void append(const T &x)
    {
        this->push_back(x);
    }
private:
    omp_lock_t writelock;
};


#include "safevector.hpp"

#endif // SAFEVECTOR_H
