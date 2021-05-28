#ifndef LINK_H
#define LINK_H
#include <string>
#include <map>
#include "Quan.h"
#include "Object.h"

using namespace std;

class Block;
class System;

class Link: public Object
{
    public:
        Link();
        Link(System *parent);
        virtual ~Link();
        Link(const Link& other);
        Link& operator=(const Link& other);
        Object* GetConnectedBlock(Expression::loc l) 
        {
            if (l == Expression::loc::source)
                return Get_s_Block();
            if (l == Expression::loc::destination)
                return Get_e_Block();
            return nullptr;
        };
        string toCommand();
        vector<string> GetAllRequieredStartingBlockProperties(); 
        vector<string> GetAllRequieredDestinationBlockProperties();
        bool ShiftLinkedBlock(int shift, Expression::loc loc);
    protected:

    private:


};

#endif // LINK_H
