#ifndef BLOCK_H
#define BLOCK_H
#include <map>
#include <string>
#include "Expression.h"
#include "Quan.h"
#include "Object.h"
#include "safevector.h"

using namespace std;

class System;
class Link;
class Reaction;

class Block: public Object
{
    public:
        Block();
        Block(System *parent);
        Block(const Block& other);
        Block& operator=(const Block& rhs);
        virtual ~Block();
        void AppendLink(int i, const Expression::loc &loc);
        double GetInflowValue(const string &variable,const Expression::timing &t);
        double GetInflowValue(const string &variable, const string &constituent, const Expression::timing &tmg);
		void shiftlinkIds(int i);
        bool deletelinkstofrom(const string& linkname="_all"); //deletes a specific links from the list of links to and from the block
        SafeVector<Link*> GetLinksFrom();
        SafeVector<Link*> GetLinksTo();
        void ClearLinksToFrom() {
            links_from_ids.clear(); links_to_ids.clear();
        }
        bool isrigid(const string& variable) { return Variable(variable)->isrigid(); }
        vector<Quan*> GetAllConstituentProperties(const string &s);
        CVector GetAllConstituentVals(const string &s, Expression::timing t);
        CVector GetAllReactionRates(vector<Reaction> *rxns, Expression::timing t);
        CVector GetAllReactionRates(Expression::timing t);
        double GetAvgOverLinks(const string& variable,const Expression::timing &tmg);
        void SetAllowLimitedFlow(bool allow)
        {
            allow_limited_flow = allow;
        };
        bool AllowLimitedFlow() {return allow_limited_flow;}
    protected:

    private:
        SafeVector<int> links_from_ids;
        SafeVector<int> links_to_ids;
        vector<string> corresponding_inflow_var;
        bool corresponding_inflow_var_extracted = false;
        bool allow_limited_flow = true;
};

#endif // BLOCK_H
