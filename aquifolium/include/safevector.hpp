#include "safevector.h"
#include "iostream"

using namespace std; 

template<class T>
SafeVector<T>::SafeVector() : vector<T> ()
{

}

template<class T>
T& SafeVector<T>::operator[](int i)
{


    if (i>int(this->size())-1)
    {   cout<<int(this->size());
        cout<<"Exceeded the size"<<std::endl;
        T x;
        return x;
    }
    else if (i<0)
    {
        cout<<"Counter is negative!"<<std::endl;
        T x;
        return x;
    }
    else
        return vector<T>::operator[](i);
}

template<class T>
unsigned int SafeVector<T>::lookup(const T &x)
{
    for (unsigned int i=0; i<this->size(); i++)
    {
        if (this->at(i) == x)
            return i;
    }
    return -1;
}


