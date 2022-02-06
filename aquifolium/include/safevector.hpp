#include "safevector.h"
#include "iostream"

template<class T>
SafeVector<T>::SafeVector() : vector<T> ()
{

}

template<class T>
T& SafeVector<T>::operator[](int i)
{
    if (i>this->size()-1)
    {   cout<<"Exceeded the size"<<endl;
        T x(0);
        return x;
    }
    else
        return vector<T>::operator[](i);
}
