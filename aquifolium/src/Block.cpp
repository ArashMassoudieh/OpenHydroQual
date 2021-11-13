#include "Block.h"
#include "Link.h"
#include "System.h"
#include "reaction.h"

Block::Block() : Object::Object()
{
    //ctor
}

Block::~Block()
{
    //dtor
}

Block::Block(const Block& other):Object::Object(other)
{
    Object::operator=(other);
}

Block& Block::operator=(const Block& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    return *this;
}


void Block::AppendLink(int i, const Expression::loc &loc)
{
    if (loc==Expression::loc::source)
        links_from_ids.push_back(i);

    if (loc==Expression::loc::destination)
        links_to_ids.push_back(i);
}

double Block::GetInflowValue(const string &variable, const Expression::timing &tmg)
{
    double inflow = 0;
    corresponding_inflow_var = Variable(variable)->GetCorrespondingInflowVar();


    for (unsigned int i=0; i<corresponding_inflow_var.size(); i++)
    {
        if (corresponding_inflow_var[i] != "")
        {
            if (Variable(corresponding_inflow_var[i]))
            {
                double inflow1 = CalcVal(corresponding_inflow_var[i]);
                Variable(corresponding_inflow_var[i])->SetVal(inflow1, tmg);
                inflow += inflow1;
            }
        }
    }
    return inflow;
}

double Block::GetInflowValue(const string &variable, const string &constituent, const Expression::timing &tmg)
{
    string variablefullname = constituent+":"+variable;
    double inflow=0;
    for (unsigned int i=0; i<Variable(variablefullname)->GetCorrespondingInflowVar().size(); i++)
    {
        if (Variable(variablefullname)->GetCorrespondingInflowVar()[i] != "")
        {
            if (Variable(constituent+":"+Variable(variablefullname)->GetCorrespondingInflowVar()[i]))
            {
                if (Variable(constituent+":"+Variable(variablefullname)->GetCorrespondingInflowVar()[i])->GetType()==Quan::_type::source)
                    if (Variable(constituent+":"+Variable(variablefullname)->GetCorrespondingInflowVar()[i])->GetSource()!=nullptr)
                    {
                        SetCurrentCorrespondingConstituent(constituent);
                        SetCurrentCorrespondingSource(Variable(constituent+":"+Variable(variablefullname)->GetCorrespondingInflowVar()[i])->GetSource()->GetName());
                    }
                double inflow1 = CalcVal(constituent+":"+Variable(variablefullname)->GetCorrespondingInflowVar()[i]);
                Variable(constituent+":"+Variable(variablefullname)->GetCorrespondingInflowVar()[i])->SetVal(inflow1, tmg);
                inflow += inflow1;
            }
        }
    }
    return inflow;
}

void Block::shiftlinkIds(int i)
{
    for (unsigned int j = 0; j < links_from_ids.size(); j++)
        if (links_from_ids[j] > i) links_from_ids[j]--; 

    for (unsigned int j = 0; j < links_to_ids.size(); j++)
        if (links_to_ids[j] > i) links_to_ids[j]--;
}

bool Block::deletelinkstofrom(const string& linkname)
{
    if (linkname == "_all")
    {
        links_from_ids.clear();
        links_to_ids.clear();
        return true; 
    }
    for (unsigned int i = 0; i < links_from_ids.size(); i++)
        if (GetLinksFrom()[i]->GetName() == linkname)
        {
            links_from_ids.erase(links_from_ids.begin() + i);
            return true;
        }

    for (unsigned int i = 0; i < links_to_ids.size(); i++)
        if (GetLinksTo()[i]->GetName() == linkname)
        {
            links_to_ids.erase(links_to_ids.begin() + i);
            return true;
        }

    return false; 
}

/*bool Block::AddProperty(const string &s, double val)
{
    if (properties.count(s)>0)
    {
        last_error = "property '" + s + "' already exists!";
        last_operation_success = false;
        return false;
    }

    if (var.count(s)>0)
    {
        last_error = "quantity '" + s + "' already exists!";
        last_operation_success = false;
        return false;
    }
    else
    {
        properties[s] = val;
        last_operation_success = true;
        return true;
    }

}*/



/*double Block::GetProperty(const string& s)
{
    if (properties.count(s)>0)
    {
        return properties[s];
        last_operation_success = true;
    }
    else
    {
        last_error = "property '" + s + "' does not exist!";
        last_operation_success = false;
        return 0;
    }
}*/

vector<Link*> Block::GetLinksFrom() {
    vector<Link* > v;
    for (unsigned int i=0; i<links_from_ids.size(); i++)
        v.push_back(Parent()->link(links_from_ids[i]));

    return v;
}
vector<Link*> Block::GetLinksTo() {
    vector<Link* > v;
    for (unsigned int i=0; i<links_to_ids.size(); i++)
        v.push_back(Parent()->link(links_to_ids[i]));

    return v;

}

vector<Quan*> Block::GetAllConstituentProperties(const string &s)
{
    vector<Quan*> out;
    for (unsigned int i=0; i<GetParent()->ConstituentsCount(); i++)
    {
        out.push_back(Variable(Parent()->constituent(i)->GetName() + ":" + s));
    }
    return out;
}

CVector Block::GetAllConstituentVals(const string &s, Expression::timing t)
{
    CVector out;
    for (unsigned int i=0; i<GetParent()->ConstituentsCount(); i++)
    {
        out.append(Variable(Parent()->constituent(i)->GetName() + ":" + s)->GetVal(t));
    }
    return out;
}

CVector Block::GetAllReactionRates(vector<Reaction> *rxns, Expression::timing t)
{
    CVector out(GetParent()->ConstituentsCount());
    for (unsigned int i=0; i<GetParent()->ReactionsCount(); i++)
    {
        double rate = rxns->at(i).RateExpression()->calc(this,t);
        for (unsigned int j=0; j<GetParent()->ConstituentsCount(); j++)
            out[j] += rate*rxns->at(i).Stoichiometric_Constant(GetParent()->constituent(j)->GetName())->calc(this,t);
    }
    return out;
}

CVector Block::GetAllReactionRates(Expression::timing t)
{
    CVector out(GetParent()->ConstituentsCount());
    for (unsigned int i=0; i<GetParent()->ReactionsCount(); i++)
    {
        double rate = GetParent()->reaction(i)->RateExpression()->calc(this,t);
        for (unsigned int j=0; j<GetParent()->ConstituentsCount(); j++)
            out[j] += rate*GetParent()->reaction(i)->Stoichiometric_Constant(GetParent()->constituent(j)->GetName())->calc(this,t)*GetVal("Storage",t);
    }
    return out;
}




