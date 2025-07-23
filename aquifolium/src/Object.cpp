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


#include "Object.h"
#include "System.h"
#include <string>
#ifdef Q_GUI_SUPPORT
#include <qdebug.h>
#include "XString.h"
#endif

Object::Object() : QuanSet()
{
    //ctor
}

Object::~Object()
{
    //dtor
}

Object::Object(const Object& other) : QuanSet(other) // Initialize base class QuanSet with other.var
{
    
    setting = other.setting;
    name = other.GetName();
    parent = other.GetParent();
	s_Block_no = other.s_Block_no;
	e_Block_no = other.e_Block_no;
	outflowlimitfactor_current = 1;
	outflowlimitfactor_past = 1;
	type = other.type;
	limitoutflow = false;
    primary_key = other.primary_key;
    Object_Type = other.Object_Type;
	SetAllParents();
}

Object& Object::operator=(const Object& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    setting = rhs.setting;
    name = rhs.GetName();
    parent = rhs.GetParent();
    QuanSet::operator=(rhs);
	s_Block_no = rhs.s_Block_no;
	e_Block_no = rhs.e_Block_no;
	type = rhs.type;
    primary_key = rhs.primary_key;
    SetAllParents();
	outflowlimitfactor_current = 1; 
	outflowlimitfactor_past = 1;
    limitoutflow = false;
    Object_Type = rhs.Object_Type;
    return *this;
}

double Object::CalcVal(const string& s,const Timing &tmg)
{
    if (Find(s))
    {
        #ifdef Debug_mode
        //ShowMessage(string("Object: ") + name + " Variable: " + s + " Value: " + numbertostring(var[s].CalcVal(tmg)));
        #endif // Debug_mode
		return (*this)[s].CalcVal(tmg);
    }
    else
    {
        if (parent)
#pragma omp critical (error_append_in_object)
            Parent()->errorhandler.Append(GetName(),"Object","CalcVal","property '" + s + "' does not exist in '" + GetName() + "'" ,1001);
        last_operation_success = false;
        return 0;
    }

}

void Object::SetQuanPointers()
{
    QuanSet::SetQuanPointers();
}
double Object::GetVal(const string& s,const Timing &tmg, bool limit)
{
    if (Find(s))
    {
        if (!limit || !Var(s).ApplyLimit())
            return Var(s).GetVal(tmg);
        else
            return Var(s).GetVal(tmg)*GetOutflowLimitFactor(tmg);
    }
    if (Parent())
    {   if (Parent()->constituent(s)!=nullptr && Count(s+":concentration")==1)
        {
            return aquiutils::Pos(Var(s+":concentration").GetVal(tmg));
        }
        if (Parent()->reactionparameter(s)!=nullptr)
        {
            if (Count(s+":value"))
            {
                return Var(s+":value").GetVal(tmg);
            }
            else
                return Parent()->reactionparameter(s)->GetVal("value",tmg);
        }
        if (Parent()->source(GetCurrentCorrespondingSource())!=nullptr)
        {
            if (Parent()->source(GetCurrentCorrespondingSource())->HasQuantity(s))
                return Parent()->source(GetCurrentCorrespondingSource())->GetVal(s,tmg);
        }
        if (HasQuantity(GetCurrentCorrespondingConstituent()+":"+s))
        {
            return aquiutils::Pos(Var(GetCurrentCorrespondingConstituent()+":"+s).GetVal(tmg));
        }
        if (Parent()->reaction(GetCurrentCorrespondingConstituent())!=nullptr)
        {
            if (Parent()->reaction(GetCurrentCorrespondingConstituent())->HasQuantity(s))
                return Parent()->reaction(GetCurrentCorrespondingConstituent())->Variable(s)->CalcVal(this,tmg);
        }

        if (parent->constituent(aquiutils::split(s,':')[0]) && aquiutils::split(s,':').size()==2)
        {
            if (parent->constituent(aquiutils::split(s,':')[0])->HasQuantity(aquiutils::split(s,':')[1]))
            {
                return parent->constituent(aquiutils::split(s,':')[0])->GetVal(aquiutils::split(s,':')[1],tmg);
            }
        }
        if (parent->reaction(aquiutils::split(s,':')[0]) && aquiutils::split(s,':').size()==2)
        {
            if (parent->reaction(aquiutils::split(s,':')[0])->HasQuantity(aquiutils::split(s,':')[1]))
            {
                return parent->reaction(aquiutils::split(s,':')[0])->GetVal(aquiutils::split(s,':')[1],tmg);
            }
        }
        else
        {
#pragma omp critical(error_handle)
            {   Parent()->errorhandler.Append(GetName(), "Object", "GetVal", "property '" + s + "' does not exist in '" + GetName() + "'", 1002);
                last_operation_success = false;
            }
        return 0;
        }
    }
    return 0;
}

double Object::GetVal(const string& variable, const string& consttnt, const Timing &tmg, bool limit)
{
    string fullname = consttnt+":"+variable;
    if (Find(fullname))
    {
        if (!limit)
        {
            if (Var(fullname).Value_Updated())
                return Var(fullname).GetVal(tmg);
            else
            {
              double val;
//#pragma omp critical(variable_change)
              val = Var(fullname).GetVal(tmg);
              return val;
            }

        }
        else
        {
            if (Var(fullname).Value_Updated())
                return Var(fullname).GetVal(tmg)*GetOutflowLimitFactor(tmg);
            else
            {
              double val;
//#pragma omp critical(variable_change_2)
              val = Var(fullname).GetVal(tmg)*GetOutflowLimitFactor(tmg);
              return val;
            }


        }
    }
    else
    {
#pragma omp critical (error_message_in_object2)
        Parent()->errorhandler.Append(GetName(),"Object","GetVal","property '" + fullname + "' does not exist in '" + GetName() + "'" ,1017);
        last_operation_success = false;
        return 0;
    }

}



double Object::GetVal(Quan* quan,const Timing &tmg, bool limit)
{
    if (!limit || !quan->ApplyLimit())
    {
        if (quan->Value_Updated())
            return quan->GetVal(tmg);
        else
        {
            double val=0;
//#pragma omp critical(getval_quan)
            val = quan->GetVal(tmg);
            return val;
        }

    }
    else
    {   if (quan->Value_Updated())
            return quan->GetVal(tmg);
        else
        {
            double val=0;
//#pragma omp critical(getval_quan_1)
            val = quan->GetVal(tmg)*GetOutflowLimitFactor(tmg);
            return val;
        }
    }
}




bool Object::AddQnantity(const string &name,const Quan &Q)
{
    if (find(name)!=end() && !size())
    {
        Parent()->errorhandler.Append(GetName(),"Object","AddQnantity","Variable " + name + " already exists! ",1003);
        return false;
    }
    else
    {
		Append(name, Q);
        return true;
    }

}

bool Object::SetQuantities(QuanSet &Q)
{
    if (Q.Count("name")==0)
    {
        if (Parent())
            Parent()->errorhandler.Append(GetName(),"Object","AddQnantity","Variable " + name + " does not exists! ",1043);
        return false;
    }
    else
    {
        QuanSet::operator=(Q);
        QuanSet::SetParent(this);
        SetName(Q["name"].GetProperty());
        return true;
    }

}

bool Object::SetQuantities(MetaModel &m, const string& typ )
{
    if (m.Count(typ)==0)
    {
        if (Parent())
            Parent()->errorhandler.Append(GetName(),"Object","SetQuantities","Type " + typ + "was not found!",1004);
        last_error = "Type " + typ + "was not found";
		return false;
	}
    else
        QuanSet::operator=(*m[typ]);
    SetDefaults();
    for (unordered_map<string, Quan>::const_iterator s = begin(); s != end(); ++s)
        Var(s->first).SetParent(this);

    return true;
}

bool Object::SetQuantities(MetaModel *m, const string& typ )
{
    if (m->Count(typ)==0)
    {
        if (Parent())
            Parent()->errorhandler.Append(GetName(),"Object","SetQuantities","Type " + typ + "was not found!",1004);
        last_error = "Type " + typ + "was not found";
        return false;
    }
    else
        QuanSet::operator=(*(m->GetItem(typ)));
    SetDefaults();
    for (unordered_map<string, Quan>::const_iterator s = begin(); s != end(); ++s)
        Var(s->first).SetParent(this);
    return true;
}

bool Object::SetQuantities(System *sys, const string& typ )
{
    SetType(typ);
    MetaModel *m = sys->GetMetaModel();
    if (m->Count(typ)==0)
    {
        if (Parent())
            Parent()->errorhandler.Append(GetName(),"Object","SetQuantities","Type " + typ + "was not found!",1004);
        last_error = "Type " + typ + "was not found";
        return false;
    }
    else
        QuanSet::operator=(*(m->GetItem(typ)));
    SetDefaults();
    for (unordered_map<string, Quan>::const_iterator s = begin(); s != end(); ++s)
        Var(s->first).SetParent(this);
    return true;
}

void Object::SetDefaults()
{
    for (unordered_map<string, Quan>::const_iterator s = begin(); s != end(); ++s)
    {
        //qDebug() << "Setting Defults for variable " << QString::fromStdString(s->first);
        if (Var(s->first).Default() != "")
        {
            //qDebug()<<QString::fromStdString(s->first);
            Var(s->first).SetProperty(Var(s->first).Default().c_str(),true);
            //qDebug()<<"Done!";
            //qDebug() << "Default Value was set to " << QString::fromStdString(var[s->first].Default());
        }
    }
}

bool Object::SetVal(const string& s, double value, const Timing &tmg)
{

    if (find(s)!=end())
    {
        Var(s).SetVal(value,tmg);
        return true;
    }
    else
    {
#pragma omp critical (setval_error)
        {
            if (Parent())
            {
               Parent()->errorhandler.Append(GetName(),"Object","SetVal","Variable " + s + " was not found!",1005);
                last_error = "Variable " + s + " was not found!";
            }
        }
        return false;
    }
}

bool Object::SetVal(const string& s, const string & value, const Timing &tmg)
{
    if (find(s)!=end())
    {
        Var(s).SetVal(aquiutils::atof(value),tmg);
        return true;
    }
    else
    {
#pragma omp critical (setval_error)
        {
        if (Parent())
            {    Parent()->errorhandler.Append(GetName(),"Object","SetVal","Variable " + s + " was not found!",1006);
                 last_error = "Variable " + s + " was not found!";
            }
        }
        return false;
    }
}

string Object::GetName() const
{
    return name;
}

bool Object::SetName(const string &s, bool setprop)
{
    if (Object_Type == object_type::constituent || Object_Type == object_type::reaction_parameter)
    {
        if (s.find('(') != std::string::npos || s.find(')') != std::string::npos)
        {
            return false; 
        }
    }

    if (setprop)
    {
        if (Count("name")>0)
            Var("name").SetProperty(s);
    }
    name = s;
    return true;
}


Object* Object::GetConnectedBlock(ExpressionNode::loc l)
{
    if (l==ExpressionNode::loc::destination)
        return e_Block;
    if (l==ExpressionNode::loc::source)
        return s_Block;

    return this;

}

void Object::SetConnectedBlock(ExpressionNode::loc l, const string &blockname)
{
    if (GetParent()->block(blockname)==nullptr)
    {
        Parent()->errorhandler.Append(GetName(),"Object","SetConnectedBlock","Block '" +blockname + "' does not exist!",1008);
        last_error = "Block '" +blockname + "' does not exist!";
        GetParent()->AppendError(last_error);
    }
    else
    {
        if (l==ExpressionNode::loc::source)
        {
            s_Block = GetParent()->block(blockname);
            s_Block_no = GetParent()->blockid(blockname);
        }
        if (l==ExpressionNode::loc::destination)
        {
            e_Block = GetParent()->block(blockname);
            e_Block_no = GetParent()->blockid(blockname);
        }
    }

}

void Object::AppendError(const string &s)
{
	errors.push_back(s);
    last_error = s;
}

void Object::SetParent(System *s)
{
    parent = s;
    SetAllParents();
}

Quan* Object::CorrespondingFlowVariable(const string &s)
{
    if (!Find(Variable(s)->GetCorrespondingFlowVar()))
    {
        Parent()->errorhandler.Append(GetName(),"Object","CorrespondingFlowVariable","Variable '" + s +"' does not exist!",1009);
        return nullptr;
    }
    else
        return Variable(Variable(s)->GetCorrespondingFlowVar());
}

Quan* Object::Variable(const string &s)
{
    if (!Find(s))
    {
        //qDebug() << QString::fromStdString("In '" + name + "': " + "Variable '" + s + "' does not exist!"); 
#ifdef Debug_mode
		ShowMessage("In '" + name + "': " + "Variable '" + s + "' does not exist!");
#endif
		//Parent()->errorhandler.Append(GetName(),"Object","Variable","Variable '" + s +"' does not exist!",1010);
		return nullptr;
    }
    else
        return &Var(s);
}

Quan* Object::Variable(const string &variable, const string &constituent)
{
    string variablefullname = constituent+":"+variable;
    if (!Find(variablefullname))
    {
        //qDebug() << QString::fromStdString("In '" + name + "': " + "Variable '" + variablefullname + "' does not exist!");
#ifdef Debug_mode
        ShowMessage("In '" + name + "': " + "Variable '" + variablefullname + "' does not exist!");
#endif
        //Parent()->errorhandler.Append(GetName(),"Object","Variable","Variable '" + variablefullname +"' does not exist!",1010);
        return nullptr;
    }
    else
        return &Var(variablefullname);
}

bool Object::VerifyQuans(ErrorHandler *errorhandler)
{
    bool fine = true; 
    for (unordered_map<string,Quan>::iterator it = begin(); it!=end(); it++)
    {
        if (!it->second.Validate())
        {
            errorhandler->Append(GetName(),"Object","VerifyQuans","In object '" + GetName() + "', " + it->second.WarningMessage(),14001);
            fine = false;
        }
    }
    return fine; 
}

vector<Quan> Object::GetCopyofAllQuans()
{
	vector<Quan> out;
    for (unordered_map<string, Quan>::iterator it = GetVars()->begin(); it != GetVars()->end(); it++)
    {
        out.push_back(it->second);
    }
    return out;
}

bool Object::HasQuantity(const string &q) const
{
    if (Count(q)==0)
        return false;
    else
        return true;

}

bool Object::Renew(const string & variable)
{
	if (!Variable(variable))
	{
#pragma omp critical (renew)
        Parent()->errorhandler.Append(GetName(),"Object","Renew","Variable '" + variable + "' does not exist!",1011);
		return false;
	}
	else
		Variable(variable)->Renew();
	return true;

}

bool Object::Update(const string & variable)
{
	if (!Variable(variable))
    {
#pragma omp critical (update)
        Parent()->errorhandler.Append(GetName(),"Object","Update","Variable '" + variable + "' does not exist!",1011);
		return false;
	}
	else
		Variable(variable)->Update();
	return true;

}

bool Object::CalcExpressions(const Timing &tmg)
{
    for (unordered_map<string, Quan>::const_iterator s = begin(); s != end(); ++s)
		if (Var(s->first).GetType() == Quan::_type::expression)
			Variable(s->first)->SetVal(Variable(s->first)->CalcVal(tmg),tmg);
	return true; 
}

bool Object::EstablishExpressionStructure()
{
    /*for (unordered_map<string, Quan>::const_iterator s = var.begin(); s != var.end(); ++s)
        if (var[s->first].GetType() == Quan::_type::expression)
            Variable(s->first)->EstablishExpressionStructure();*/
    return true;
}

void Object::SetVariableParents()
{
	QuanSet::SetParent(this);
    for (unordered_map<string, Quan>::const_iterator s = begin(); s != end(); ++s)
	{
		Var(s->first).SetParent(this);
		if (Var(s->first).GetType() == Quan::_type::source)
            if (Var(s->first).SourceName() != "")
			    Var(s->first).SetProperty(Var(s->first).SourceName());
	}
}

void Object::SetLimitedOutflow(bool x)
{
    limitoutflow = x;
}

void Object::ShowMessage(const string &msg) {if (!parent->IsSilent()) cout<<msg<<std::endl;}

void Object::SetAllParents()
{
    QuanSet::SetParent(this);
}

bool Object::SetProperty(const string &prop, const string &value, bool force_value, bool check_criteria)
{
    if (!HasQuantity(prop))
    {
#pragma omp critical (set_property)
        {    if (Parent())
                Parent()->errorhandler.Append(GetName(),"Object","SetProperty","Object '" + GetName() + "' has no property called '" + prop + "'",1012);
        }
        return false;
    }
    if (Var(prop).GetType() == Quan::_type::value || Var(prop).GetType() == Quan::_type::balance || Var(prop).GetType() == Quan::_type::constant || (Var(prop).GetType() == Quan::_type::expression && (Var(prop).Delegate()=="UnitBox"||Var(prop).Delegate()=="ValueBox" )))
    {
#ifdef Q_GUI_SUPPORT
        if (var[prop].Delegate()=="UnitBox")
        {
            if (aquiutils::split(value,'[').size()>1)
            {   string unit = aquiutils::split(aquiutils::split(value,'[')[1],']')[0];
                double coefficient = XString::coefficient(QString::fromStdString(unit));
                double _value = atof(value.c_str())*coefficient;
                Var(prop).SetVal(_value,Timing::both, check_criteria);
                Var(prop).Unit() = unit;
            }
            else
                Var(prop).SetVal(aquiutils::atof(value),Timing::both, check_criteria);
        }
        else
#endif
        Var(prop).SetVal(aquiutils::atof(value),Timing::both);
        return true;
    }
    if (Var(prop).GetType() == Quan::_type::expression)
    {
        return Var(prop).SetExpression(value);
    }
    if (Var(prop).GetType() == Quan::_type::rule)
    {
        Parent()->errorhandler.Append(GetName(),"Object","SetProperty","In object '" + GetName() + "', property '" + prop + "' is a rule and cannot be set during the run time",1014);
        return false;
    }

    if (Var(prop).GetType() == Quan::_type::timeseries || Var(prop).GetType() == Quan::_type::prec_timeseries)
    {
        return Var(prop).SetProperty(value);
    }
    if (Var(prop).GetType() == Quan::_type::source)
    {
        return Var(prop).SetProperty(value);
    }
    if (Var(prop).GetType() == Quan::_type::string)
    {
        return Var(prop).SetProperty(value);
    }
    return false;

}

string Object::toString(int _tabs)
{
    string out = aquiutils::tabs(_tabs) + "Name: " + GetName() + "\n";
    out += aquiutils::tabs(_tabs) + "Type: " + GetType() + "\n";
    out += QuanSet::ToString(_tabs + 1);
    return out;
}

void Object::AssignRandomPrimaryKey()
{
    string s;
    for (int i=0; i<20; i++)
        s += char(rand()%(122-65) +65);
    primary_key = s;
}

string Object::toCommand()
{
    string out = "type=" + GetType() + ",";
    out += QuanSet::toCommand();
    return out;
}

QJsonObject Object::toJson(bool allvariables, bool calculatevalue)
{
    QJsonObject out = QuanSet::toJson(allvariables, calculatevalue);
    out["type"] = QString::fromStdString(type);
    if (ObjectType() == object_type::link)
    {
        out["from"] = QString::fromStdString(parent->block(s_Block_no)->GetName());
        out["to"] = QString::fromStdString(parent->block(e_Block_no)->GetName());
    }
    return out;
}

QJsonObject Object::ExpressionstoJson() const
{
    QJsonObject out = QuanSet::EquationsToJson();
    return out; 
}

string Object::toCommandSetAsParam()
{
    string out;
    out += QuanSet::toCommandSetAsParam();
    return out;
}

vector<string> Object::ItemswithOutput()
{
    vector<string> items;
    for (unordered_map<string, Quan>::iterator it = GetVars()->begin(); it != GetVars()->end(); it++)
        if (it->second.IncludeInOutput())
        {
            if (it->second.GetOutputItem() != "")
                items.push_back(it->first);
        }
    return items; 
}


unique_ptr<vector<string>> &Object::operators()
{
    if (parent)
    {
        return parent->operators;
    }


}
unique_ptr<vector<string>> &Object::functions()
{
    if (parent)
    {
        return parent->functions;
    }
}

bool Object::RenameConstituents(const string &oldname, const string &newname)
{
    Constituent* consttnt = parent->constituent(oldname);
    if (consttnt)
    {   vector<Quan> original_quans = consttnt->GetCopyofAllQuans();

        for (unordered_map<string, Quan>::iterator it = GetVars()->begin(); it != GetVars()->end(); it++)
        {
            if (it->second.GetType() == Quan::_type::expression )
            {   for (unsigned int i=0; i<original_quans.size(); i++)
                   it->second.SetExpression(it->second.GetExpression()->RenameConstituent(oldname,newname,original_quans[i].GetName()));
            }
        }
    }
    vector<string> oldfullname;
    vector<string> newfullname;
    for (unordered_map<string, Quan>::iterator it = GetVars()->begin(); it != GetVars()->end(); it++)
    {
        if (aquiutils::split(it->first,':').size()==2)
        {   if (aquiutils::split(it->first,':')[0]==oldname)
            {
                oldfullname.push_back(it->first);
                newfullname.push_back(newname + ":" + aquiutils::split(it->first,':')[1]);
            }
        }
    }
    if (GetConnectedBlock(ExpressionNode::loc::source) != nullptr)
    {
        for (unordered_map<string, Quan>::iterator it = GetConnectedBlock(ExpressionNode::loc::source)->GetVars()->begin(); it != GetConnectedBlock(ExpressionNode::loc::source)->GetVars()->end(); it++)
        {
            if (aquiutils::split(it->first, ':').size() == 2)
            {
                if (aquiutils::split(it->first, ':')[0] == oldname)
                {
                    if (aquiutils::lookup(oldfullname, it->first) == -1)
                    {
                        oldfullname.push_back(it->first);
                        newfullname.push_back(newname + ":" + aquiutils::split(it->first, ':')[1]);
                    }
                }
                if (aquiutils::split(it->first, ':')[0] == newname)
                {
                    if (aquiutils::lookup(newfullname, it->first) == -1)
                    {
                        newfullname.push_back(it->first);
                        oldfullname.push_back(oldname + ":" + aquiutils::split(it->first, ':')[1]);
                    }
                }
            }
        }
    }

    bool succeed = true;
    for (unsigned int i=0; i<oldfullname.size(); i++)
    {
        succeed &= RenameProperty(oldfullname[i],newfullname[i]);
        if (Variable(newfullname[i])!=nullptr)
        {   Variable(newfullname[i])->Description() = newname + ":" + aquiutils::split(newfullname[i],':')[1];
            Variable(newfullname[i])->Description(true) = newname + ":" + aquiutils::split(newfullname[i],':')[1];
        }
    }
    
    return succeed;
}

bool Object::CalculateInitialValues()
{
    bool succeed = true;
    for (unsigned int j = 0; j < QuantitOrder().size(); j++)
    {
        if (Variable(QuantitOrder()[j])->calcinivalue())
        {   
            double ini_value = Expression(Variable(QuantitOrder()[j])->InitialValueExpression()).calc(this,Timing::past);
            Variable(QuantitOrder()[j])->SetVal(ini_value,Timing::both);
        }
    }
    return succeed;
}

vector<string> Object::AllConstituents()
{
    if (parent)
        return parent->AllConstituents();
    else
        return vector<string>();
}

vector<string> Object::AllReactionParameters()
{
    if (parent)
        return parent->AllReactionParameters();
    else
        return vector<string>();
}

bool Object::InitializePrecalcFunctions()
{
    QuanSet::InitializePrecalcFunctions();
    return true; 
}

void Object::MakeTimeSeriesUniform(const double &increment)
{
    for (unordered_map<string, Quan>::const_iterator s = begin(); s != end(); ++s)
        
        if (Var(s->first).GetType() == Quan::_type::timeseries || Var(s->first).GetType() == Quan::_type::prec_timeseries)
        {
            if (var[s->first].TimeSeries()!=nullptr)
                *(var[s->first].TimeSeries()) = var[s->first].TimeSeries()->make_uniform(increment);
        }
}

bool Object::CopyStateVariablesFrom(Object* obj)
{
    for (unordered_map<string, Quan>::const_iterator s = begin(); s != end(); ++s)
    {
        if (Var(s->first).GetType() == Quan::_type::balance && obj->HasQuantity(s->first))
        {
            Var(s->first).SetVal(obj->GetVal(s->first, Timing::past), Timing::past);
            Var(s->first).SetVal(obj->GetVal(s->first, Timing::present), Timing::present);
        }
    }
    SetLimitedOutflow(obj->GetLimitedOutflow());
    return true;
}

unordered_map<string, Quan*> Object::AllSourceParameters()
{
    unordered_map<string, Quan* > out;
    for (unordered_map<string, Quan>::iterator s = begin(); s != end(); ++s)
    {
        if (Var(s->first).GetType() == Quan::_type::source)
        {
            if(Var(s->first).GetSource()!=nullptr)
            {
                for (unordered_map<string, Quan>::const_iterator s1 = Var(s->first).GetSource()->GetVars()->begin(); s1 != Var(s->first).GetSource()->GetVars()->end(); ++s1)
                {
                    out[s1->first] = Var(s->first).GetSource()->Variable(s1->first);
                }
            }
        }
    }
    return out;
}
