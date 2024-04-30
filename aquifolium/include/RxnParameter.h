#ifndef RXNPARAMETER_H
#define RXNPARAMETER_H

#include <Object.h>

class RxnParameter : public Object
{
public:
    RxnParameter();
    RxnParameter(System *parent);
    RxnParameter(const RxnParameter& other);
    RxnParameter& operator=(const RxnParameter& rhs);
    virtual ~RxnParameter();
    bool SetName(const string &newname, bool setprop=true);
};

#endif // RXNPARAMETER_H


#include "Object.h"
#include "Expression.h"
#include <map>

