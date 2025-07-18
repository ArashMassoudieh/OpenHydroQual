/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */


#include "Block.h"
#include "Link.h"
#include "System.h"
#include "reaction.h"

Block::Block() : Object::Object()
{
    SetObjectType(object_type::block);
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


void Block::AppendLink(int i, const ExpressionNode::loc &loc)
{
    if (loc==ExpressionNode::loc::source)
        links_from_ids.push_back(i);

    if (loc==ExpressionNode::loc::destination)
        links_to_ids.push_back(i);
}

double Block::GetInflowValue(const string &variable, const Timing &tmg)
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
                if (inflow1>0)
                    inflow += inflow1;
                else
                    inflow += inflow1*min(GetOutflowLimitFactor(tmg),1.0);
            }
        }
    }
    return inflow;
}

double Block::GetInflowValue(const string &variable, const string &constituent, const Timing &tmg)
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
                double inflow1 = CalcVal(constituent+":"+Variable(variablefullname)->GetCorrespondingInflowVar()[i],tmg);
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

SafeVector<Link*> Block::GetLinksFrom() {
    SafeVector<Link* > v;
    for (unsigned int i=0; i<links_from_ids.size(); i++)
        v.push_back(Parent()->link(links_from_ids[i]));

    return v;
}
SafeVector<Link*> Block::GetLinksTo() {
    SafeVector<Link* > v;
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

CVector Block::GetAllConstituentVals(const string &s, Timing t)
{
    CVector out;
    for (unsigned int i=0; i<GetParent()->ConstituentsCount(); i++)
    {
        out.append(Variable(Parent()->constituent(i)->GetName() + ":" + s)->GetVal(t));
    }
    return out;
}

CVector Block::GetAllReactionRates(vector<Reaction> *rxns, Timing t)
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

CVector Block::GetAllReactionRates(Timing t)
{
    CVector out(GetParent()->ConstituentsCount());
    for (unsigned int i=0; i<GetParent()->ReactionsCount(); i++)
    {
        SetCurrentCorrespondingConstituent(GetParent()->reaction(i)->GetName());
        double rate = GetParent()->reaction(i)->RateExpression()->calc(this,t);
        for (unsigned int j=0; j<GetParent()->ConstituentsCount(); j++)
            out[j] += rate*GetParent()->reaction(i)->Stoichiometric_Constant(GetParent()->constituent(j)->GetName())->calc(this,t)*GetVal(GetParent()->reaction(i)->GetVars()->Normalizing_Quantity(),t);
    }
    return out;
}

double Block::GetAvgOverLinks(const string& variable,const Timing &tmg)
{
    double sum=0;
    double count = 0;
    SafeVector<Link*> linksto = GetLinksTo();
    SafeVector<Link*> linksfrom = GetLinksFrom();
#ifndef NO_OPENMP
#pragma omp critical
#endif
{
    for (unsigned int i=0; i<linksfrom.size(); i++)
    {   if (linksfrom[i]->HasQuantity(variable))
        {
            sum+= linksfrom[i]->GetVal(variable,tmg);
            count++;
        }
    }
    for (unsigned int i=0; i<linksto.size(); i++)
    {
        if (linksto[i]->HasQuantity(variable))
        {
            sum+= linksto[i]->GetVal(variable,tmg);
            count ++;
        }
    }
}
    if (count>0)
        return sum/count;
    else
        return 0;
}






