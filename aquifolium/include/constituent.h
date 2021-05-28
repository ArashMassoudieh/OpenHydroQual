#ifndef CONSTITUENT_H
#define CONSTITUENT_H

#include "Expression.h"
#include "BTC.h"
#include "Object.h"

class System;

class Constituent: public Object
{
public:
    Constituent();
    virtual ~Constituent();
    Constituent(const Constituent& other);
    Constituent& operator=(const Constituent& other);
    bool SetName(const string &s, bool setprop=true);

};

#endif // CONSTITUENT_H
