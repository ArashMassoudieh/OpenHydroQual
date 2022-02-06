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
};

#include "safevector.hpp"

#endif // SAFEVECTOR_H
