#ifndef BLOCK_H
#define BLOCK_H
#include <map>
#include <string>
#include "Expression.h"
#include "Quan.h"
#include "Object.h"

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
        vector<Link*> GetLinksFrom();
        vector<Link*> GetLinksTo();
        void ClearLinksToFrom() {
            links_from_ids.clear(); links_to_ids.clear();
        }
        bool isrigid(const string& variable) { return Variable(variable)->isrigid(); }
        vector<Quan*> GetAllConstituentProperties(const string &s);
        CVector GetAllConstituentVals(const string &s, Expression::timing t);
        CVector GetAllReactionRates(vector<Reaction> *rxns, Expression::timing t);
        CVector GetAllReactionRates(Expression::timing t);


    protected:

    private:
        vector<int> links_from_ids;
        vector<int> links_to_ids;
};

#endif // BLOCK_H
