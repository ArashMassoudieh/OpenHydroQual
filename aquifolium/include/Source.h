#ifndef SOURCE_H
#define SOURCE_H

#include "Object.h"
#include "Quan.h"

using namespace std;

class System;
class Source: public Object
{
    public:
        Source();
        virtual ~Source();
        Source(System *_parent);
        Source(const Source& other);
        Source& operator=(const Source& rhs);
        double GetValue(Object *obj);
    protected:

    private:
};

#endif // SOURCE_H
