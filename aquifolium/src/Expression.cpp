#include "Expression.h"
//#include "utility_funcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <iostream>
#include <Block.h>
#include <fstream>
#include <sstream>
#include <cctype>
#include "System.h"
//#include "QDebug"

#define SMALLNUMBER 1e-23
using namespace std;

bool Expression::func_operators_initialized =false;
vector<string> Expression::funcs = vector<string>();
vector<string> Expression::opts = vector<string>();

Expression::Expression(void)
{
    if (Expression::func_operators_initialized != true)
    {   Expression::funcs.push_back("_min");
        Expression::funcs.push_back("_max");
        Expression::funcs.push_back("_exp");
        Expression::funcs.push_back("_log");
        Expression::funcs.push_back("_abs");
        Expression::funcs.push_back("_sgn");
        Expression::funcs.push_back("_sqr");
        Expression::funcs.push_back("_sqt");
        Expression::funcs.push_back("_lpw");
        Expression::funcs.push_back("_pos");
        Expression::funcs.push_back("_hsd");
        Expression::funcs.push_back("_ups");
        Expression::funcs.push_back("_bkw");
        Expression::funcs.push_back("_mon");
        Expression::funcs.push_back("_mbs");
        Expression::funcs.push_back("_ekr");
        Expression::opts.push_back("+");
        Expression::opts.push_back("-");
        Expression::opts.push_back("*");
        Expression::opts.push_back(";");
        Expression::opts.push_back("/");
        Expression::opts.push_back("^");
        Expression::func_operators_initialized = true;
    }


}

Expression::Expression(string S)
{
    text = S;
    if (Expression::func_operators_initialized != true)
    {   Expression::funcs.push_back("_min");
        Expression::funcs.push_back("_max");
        Expression::funcs.push_back("_exp");
        Expression::funcs.push_back("_log");
        Expression::funcs.push_back("_abs");
        Expression::funcs.push_back("_sgn");
        Expression::funcs.push_back("_sqr");
        Expression::funcs.push_back("_sqt");
        Expression::funcs.push_back("_lpw");
        Expression::funcs.push_back("_pos");
        Expression::funcs.push_back("_hsd");
        Expression::funcs.push_back("_ups");
        Expression::funcs.push_back("_bkw");
        Expression::funcs.push_back("_mon");
        Expression::funcs.push_back("_mbs");
        Expression::funcs.push_back("_ekr");
        Expression::opts.push_back("+");
        Expression::opts.push_back("-");
        Expression::opts.push_back("*");
        Expression::opts.push_back(";");
        Expression::opts.push_back("/");
        Expression::opts.push_back("^");
        Expression::func_operators_initialized = true;
    }


	vector<string> out;
	//bool inside_quote = false;
	unsigned int paranthesis_level = 0;
    int last_operator_location = -1;
	if (!aquiutils::parantheses_balance(S))
	{
		_errors.push_back("Parentheses do not match in" + S);
		return;
	}
    if (aquiutils::lookup(Expression::funcs, aquiutils::left(S,4))!=-1 )
	{
        if (aquiutils::corresponding_parenthesis(S,4) == int(S.size()-1))
        {   function = aquiutils::right(aquiutils::left(S,4),3);
            S = aquiutils::right(S, S.size() - 4);
        }
	}
	if (aquiutils::left(S,1) == "(")
	{
		if (aquiutils::corresponding_parenthesis(S,0) == -1 )
		{
			_errors.push_back("Parentheses do not match in" + S);
		}
		else if (aquiutils::corresponding_parenthesis(S,0) == int(S.size()-1))
		{
			aquiutils::remove(S,0);
			aquiutils::remove(S,S.size() - 1);
			//if (opts.contains(S.left(1)))
			//	terms.append(CExpression("0"));
            if (aquiutils::lookup(Expression::funcs, aquiutils::left(S,4))!=-1)
			{
                //function = aquiutils::right(aquiutils::left(S,4),3);
			}

		}
	}
	if (aquiutils::isnumber(S))
	{
		param_constant_expression = "constant";
		constant = aquiutils::atof(S);
	}
	else
	{
		if (aquiutils::contains(S,"(") || aquiutils::contains(S,")") || aquiutils::contains(S,"+") || aquiutils::contains(S,"-") || aquiutils::contains(S,"*") || aquiutils::contains(S,"/") || aquiutils::contains(S,"^") || aquiutils::contains(S,";"))
		{
			param_constant_expression = "expression";
			for (unsigned int i = 0; i < S.size(); i++)
			{
				if (S.substr(i, 1) == "(")
					paranthesis_level++;

				if (S.substr(i, 1) == ")")
					paranthesis_level--;

				if (paranthesis_level == 0)
					if ((S.substr(i, 1) == "+") || (S.substr(i, 1) == "-") || (S.substr(i, 1) == "*") || (S.substr(i, 1) == "/") || (S.substr(i, 1) == "^") || (S.substr(i, 1) == ";"))
					{
						operators.push_back(S.substr(i, 1));
						Expression sub_exp = Expression(aquiutils::trim(S.substr(last_operator_location+1, i -1- last_operator_location)));
						if (sub_exp.text != "")
						{
                            if (operators.size() > 1)
								sub_exp.sign = operators[operators.size() - 2];
							else
								sub_exp.sign = "+";
							terms.push_back(sub_exp);
						}
						else
						{
							sub_exp = Expression("0");
							sub_exp.sign = "+";
							terms.push_back(sub_exp);
						}
						aquiutils::push_back(_errors,sub_exp._errors);
						last_operator_location = i;
					}
			}

			
            Expression sub_exp = Expression(aquiutils::trim(S.substr(last_operator_location+1, S.size() - last_operator_location)));
			if (operators.size() > 0)
				sub_exp.sign = operators[operators.size() - 1];
			else
				sub_exp.sign = "+";
			terms.push_back(sub_exp);
			aquiutils::push_back(_errors,sub_exp._errors);
            Setup_Calculation_Structure();

		}
		else
		{
			param_constant_expression = "parameter";
			if (aquiutils::split(S,'.').size()==1)
			{   parameter = S;
                location = loc::self;
			}
			else if (aquiutils::split(S,'.').size()==2)
            {
                if (aquiutils::tolower(aquiutils::split(S,'.')[1]) == "s")
                {
                    parameter = aquiutils::split(S,'.')[0];
                    location = loc::source;
                }
                if (aquiutils::tolower(aquiutils::split(S,'.')[1]) == "e")
                {
                    parameter = aquiutils::split(S,'.')[0];
                    location = loc::destination;
                }
                if (aquiutils::tolower(aquiutils::split(S,'.')[1]) == "v")
                {
                    parameter = aquiutils::split(S,'.')[0];
                    location = loc::average_of_links;
                }
            }
		}
	}


}

Expression::Expression(const Expression & S)
{
    operators.clear();
    term_vals.clear();
	terms_calculated.clear();
	_errors.clear(); 
	quan = nullptr;
	operators = S.operators;
	constant = S.constant;
	terms = S.terms;
	sign = S.sign;
	function = S.function;
	parameter = S.parameter;
	param_constant_expression = S.param_constant_expression;
	unit = S.unit;
	text = S.text;
	location = S.location;
    term_sources.clear();
    term_sources_determined = false;
    sourceterms_resized = false;
    CalculationStructure = S.CalculationStructure;

}

Expression & Expression::operator=(const Expression &S)
{
    terms.clear();
	term_vals.clear();
	terms_calculated.clear();
	_errors.clear();
	quan = nullptr;
	operators = S.operators;
	constant = S.constant;
	terms = S.terms;
	sign = S.sign;
    function = S.function;
	parameter = S.parameter;
	param_constant_expression = S.param_constant_expression;
	unit = S.unit;
	text = S.text;
    location = S.location;
    term_sources.clear();
    term_sources_determined = false;
    sourceterms_resized = false;
    CalculationStructure = S.CalculationStructure;
	return *this;
}


Expression::~Expression(void)
{
    _errors.clear();
	terms.clear();
	term_sources.clear();
	term_vals.clear();
	terms_calculated.clear();
}

vector<string> Expression::extract_operators(string s)
{
	vector<string> out;
	//bool inside_quote = false;
	int paranthesis_level = 0;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s.substr(i, 1) == "(")
			paranthesis_level++;

		if (s.substr(i, 1) == ")")
			paranthesis_level--;

		if (paranthesis_level == 0)
			if ((s.substr(i, 1) == "+") || (s.substr(i, 1) == "-") || (s.substr(i, 1) == "*") || (s.substr(i, 1) == "/") || (s.substr(i, 1) == "^")) out.push_back(s.substr(i, 1));


	}

	return out;
}

vector<string> Expression::extract_terms(string s)
{
	bool inside_quote = false;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s.substr(i, 1) == "'")
		{
			inside_quote = !inside_quote;
			aquiutils::remove(s, i);
		}
		if (inside_quote)
			if (s.substr(i, 1) == " ") aquiutils::replace(s, i, 1, "|");

	}
	vector<string> out = aquiutils::split(s,' ');
	for (unsigned int i = 0; i < out.size(); i++) aquiutils::replace(out[i],"|", " ");
	return out;
}


void Expression::EstablishSourceStructure()
{
    if (!sourceterms_resized)
    {
        {
            term_sources.resize(terms.size());
            for (unsigned int j=0; j<terms.size(); j++)
                term_sources[j].resize(terms.size());
            terms_source_counter.resize(terms.size());
        }
    }
    sourceterms_resized = true;
}


void Expression::ResetTermsSources()
{

    for (unsigned int i=0; i<term_sources.size(); i++)
    {   for (unsigned int j=0; j<term_sources[i].size(); j++)
            term_sources[i][j] = -1;
        terms_source_counter[i]=0;
    }

}



bool Expression::SetQuanPointers(Object *W)
{
    if (param_constant_expression=="parameter")
    {   if (location==Expression::loc::self)
            quan = W->Variable(parameter);
        else if (location!=Expression::loc::average_of_links && W->ObjectType()==object_type::link)
            quan = W->GetConnectedBlock(location)->Variable(parameter);
        else
            quan = nullptr;
        return true;
    }
    else if (param_constant_expression == "expression")
    {
        for (unsigned int i=0; i<terms.size(); i++)
            terms[i].SetQuanPointers(W);
        return true;
    }

    return false;

}

double Expression::calc(Object *W, const timing &tmg, bool limit)
{
	if (!W)
	{
		cout << "Pointer is empty!" << std::endl;
		return 0;
	}



    EstablishSourceStructure();
    ResetTermsSources();

	if (function=="ups")
	{
        if (terms.size()!=2)
        {
            W->Parent()->errorhandler.Append(W->GetName(),"Expression","calc","Function 'ups' requiers two arguments", 7001);
            return 0;
        }
        else if ((W->GetConnectedBlock(Expression::loc::source)==nullptr) || (W->GetConnectedBlock(Expression::loc::destination)==nullptr))
        {
            W->Parent()->errorhandler.Append(W->GetName(),"Expression","calc","When function 'ups' is used the object must have source and destination", 7002);
            return 0;
        }
        else
        {
            double flow = terms[0].calc(W,tmg,limit);
            if (flow>0)
            {
                return flow*terms[1].calc(W->GetConnectedBlock(Expression::loc::source),tmg,limit);
            }
            else
            {
                return flow*terms[1].calc(W->GetConnectedBlock(Expression::loc::destination),tmg,limit);
            }
        }
	}

    if (function=="bkw")
    {
        if (terms.size()!=2)
        {
            W->Parent()->errorhandler.Append(W->GetName(),"Expression","calc","Function 'bkw' requiers two arguments", 7001);
            return 0;
        }
        else if ((W->GetConnectedBlock(Expression::loc::source)==nullptr) || (W->GetConnectedBlock(Expression::loc::destination)==nullptr))
        {
            W->Parent()->errorhandler.Append(W->GetName(),"Expression","calc","When function 'bkw' is used the object must have source and destination", 7002);
            return 0;
        }
        else
        {
            double slope = terms[0].calc(W->GetConnectedBlock(Expression::loc::source),tmg,limit) - terms[0].calc(W->GetConnectedBlock(Expression::loc::destination),tmg,limit);
            if (slope>0)
            {
                return terms[1].calc(W->GetConnectedBlock(Expression::loc::source),tmg,limit);
            }
            else
            {
                return terms[1].calc(W->GetConnectedBlock(Expression::loc::destination),tmg,limit);
            }
        }
    }
    if (function=="ekr")
    {
        if (terms.size()!=2)
        {
            W->Parent()->errorhandler.Append(W->GetName(),"Expression","calc","Function 'ekr' requiers two arguments", 7002);
            return 0;
        }
        if (!W->HasQuantity(terms[0].parameter))
        {
            W->Parent()->errorhandler.Append(W->GetName(),"Expression","calc","Block '"+W->GetName()+ "' has no property " + terms[0].parameter, 7003);
            return 0;
        }
        if (W->Variable(terms[0].parameter)->GetType()!=Quan::_type::prec_timeseries && W->Variable(terms[0].parameter)->GetType()!=Quan::_type::timeseries)
        {
            W->Parent()->errorhandler.Append(W->GetName(),"Expression","calc","In block '"+W->GetName()+ "' property '" + terms[0].parameter + "' must be of type time-series", 7003);
            return 0;
        }
        if (W->Variable(terms[0].parameter)->TimeSeries())
            return W->Variable(terms[0].parameter)->TimeSeries()->Exponential_Kernel(W->Parent()->GetTime(),terms[1].calc(W,tmg,limit));
        else
            return 0;
    }

	if (param_constant_expression == "constant")
		return constant;
	if (param_constant_expression == "parameter")
	{
       double out=0;

       if (location == loc::self)
        {
            if (quan!=nullptr && quan->GetParent()==W)
                out = W->GetVal(quan, tmg,limit);
            else
                out = W->GetVal(parameter, tmg,limit);
        }
        else if (location!=loc::average_of_links)
        {
            if (W->GetConnectedBlock(location)!=nullptr)
            {    if (quan!=nullptr && quan->GetParent()==W->GetConnectedBlock(location))
                    out = W->GetConnectedBlock(location)->GetVal(quan, tmg, limit);
                 else
                    out = W->GetConnectedBlock(location)->GetVal(parameter, tmg, limit);
            }
            else
            {   if (quan!=nullptr && quan->GetParent()==W)
                    out = W->GetVal(quan, tmg, limit);
                else
                    out = W->GetVal(parameter, tmg, limit);
            }
        }
        else
       {
            if (W->ObjectType()==object_type::block)
            {
                out = dynamic_cast<Block*>(W)->GetAvgOverLinks(parameter,tmg);
            }
       }

        if (function == "")
            return out;
        else if (count_operators(";")==0)
            return func(function, out);
	}
	if (param_constant_expression == "expression")
	{

        //qDebug()<<QString::fromStdString(this->ToString())<<":"<<term_sources_determined;

        for (unsigned int i = 0; i < terms.size(); i++)
        {
            term_sources[i][0]=i;
            terms_source_counter[i]++;
        }
        if (!term_sources_determined)
        {   term_vals.resize(terms.size());
            terms_calculated.resize(terms.size());
        }

        for (unsigned int i = 0; i < terms.size(); i++) terms_calculated[i]=false;

        for (int i = operators.size() - 1; i >= 0; i--)
		{
			if (operators[i] == "^")
				oprt(operators[i], i, i + 1, W, tmg, limit);
		}
		for (int i = operators.size() - 1; i >= 0; i--)
		{
			if (operators[i] == "*")
				oprt(operators[i], i, i + 1, W, tmg, limit);
		}

		for (int i = operators.size() - 1; i >= 0; i--)
		{
			if (operators[i] == "/")
				oprt(operators[i], i, i + 1, W, tmg,limit);
		}

		for (int i = operators.size() - 1; i >= 0; i--)
		{
			if (operators[i] == "+")
				oprt(operators[i], i, i + 1, W, tmg,limit);

		}

		for (int i = operators.size() - 1; i >= 0; i--)
		{
			if (operators[i] == "-")
			{
				oprt(operators[i], i, i + 1, W, tmg,limit);
			}
		}
        if (operators.size()==0)
        {
            term_vals[0]=terms[0].calc(W,tmg,limit);
        }
        unsigned int seploc=0;
        for (int i = operators.size() - 1; i >= 0; i--)
        {
            if (operators[i] == ";")
            {
                seploc = i;
                if (i==0)
                    term_vals[0] = terms[0].calc(W,tmg,limit);
                if (i==int(operators.size()-1))
                    term_vals[i+1] = terms[i+1].calc(W,tmg,limit);

            }
        }
        term_sources_determined = true;
		if (function == "")
			return term_vals[0];
		else if (count_operators(";")==0)
			return func(function, term_vals[0]);
		else if (count_operators(";")==1)
            return func(function, term_vals[seploc], term_vals[seploc+1]);
        else if (count_operators(";")==2)
            return func(function, term_vals[0], term_vals[1], term_vals[2]);

    }

    return 0;
}


double Expression::func(string &f, double val)
{

	if (f == "exp")
		return exp(val);
	if (f == "log")
    {	if (val>0)
            return log(val);
        else
            return -1e12;
    }
	if (f == "abs")
		return fabs(val);
	if (f == "sgn")
		return (val>0?+1:-1);
	if (f == "sqr")
        return sqrt(aquiutils::Pos(val));
    if (f == "sqt")
    {	if (val>0)
            return sqrt(val*val/(fabs(val)+1e-4));
        else
            return -sqrt(val*val/(fabs(val)+1e-4));
    }
    if (f == "pos")
        return (val+fabs(val))/2.0;
    if (f == "hsd")
    {
        if (val>=0) return 1; else return 0;
    }
	return val;
}

double Expression::func(string &f, double val1, double val2)
{
	if (f == "min")
		return min(val1, val2);
	if (f == "max")
		return max(val1, val2);
	if (f == "mon")
		return val1 / (val1 + val2);
	if (f == "mbs")
		return abs(val1) / (abs(val1) + val2);
	if (f == "lpw")
		return pow(fabs(val1),1-(1-val2)*val1/(1e-6+val1));
	return val1;
}

double Expression::func(string &f, double cond, double val1, double val2)
{
	if (f == "ups")
	{
        if (cond>=0)
            return val1;
        else
            return val2;
	}
    else if (f=="bkw")
    {
        if (cond>=0)
            return val1;
        else
            return val2;
    }

	return val1;
}


double Expression::oprt(string &f, double val1, double val2)
{
    if (f == "^") return
        pow(aquiutils::Pos(val1), val2);
    if (f == "+") return
        val1 + val2;
    if (f == "-") return
        val1 + val2;
    if (f == "/") return
        val1 * val2;
    if (f == "*") return
        val1*val2;
	return 0;
}

double Expression::oprt(string &f, unsigned int i1, unsigned int i2, Object *W, const Expression::timing &tmg, bool limit)
{

    for (unsigned int j = 0; j < terms_source_counter[i1]; j++)
    {
        if (term_sources.size() > i2)
            for (unsigned int k=0; k<terms_source_counter[i2]; k++)
            {   if (term_sources[i2][k]!=-1 && term_sources[i1][j]!=-1)
                {   vector<int> vec = term_sources[term_sources[i2][k]];
                    if (aquiutils::lookup(vec,term_sources[i1][j])==-1)
                    {   term_sources[term_sources[i2][k]][terms_source_counter[term_sources[i2][k]]]=term_sources[i1][j];
                        terms_source_counter[term_sources[i2][k]]++;
                    }
                }
            }

    }
    if (term_sources.size() > i2)
        for (unsigned int j = 0; j < terms_source_counter[i2]; j++)
        {
            for (unsigned int k = 0; k<terms_source_counter[i1]; k++)
            {
                if (term_sources[i1][k]!=-1 && term_sources[i2][j]!=-1)
                {
                    if (aquiutils::lookup(term_sources[term_sources[i1][k]],term_sources[i2][j])==-1)
                    {
                        term_sources[term_sources[i1][k]][terms_source_counter[term_sources[i1][k]]] = term_sources[i2][j];
                        terms_source_counter[term_sources[i1][k]]++;
                    }
                }
            }

        }



    double val1;
	double val2;
	if (terms_calculated[i1]) val1 = term_vals[i1]; else val1 = terms[i1].calc(W, tmg, limit);
	if (!(i1>0 && terms_calculated[i1]))
        if (terms[i1].sign == "/")
            val1 = 1/(val1+1e-23);
	if (terms[i1].sign == "-") val1 = -val1;
    if (term_sources.size() > i2)
		if (terms_calculated[i2]) val2 = term_vals[i2]; else
		{
			val2 = terms[i2].calc(W, tmg, limit);
			if (terms[i2].sign == "/") val2 = 1 / (val2+1e-23);
			if (terms[i2].sign == "-") val2 = -val2;
		}
	else
	{
		val1 = 0;
		val2 = val1;
	}

	if (unit == "")
	{
		if (terms[i1].unit != "") unit = terms[i1].unit;
		else if (terms[i2].unit != "") unit = terms[i2].unit;
	}

    for (unsigned int j = 0; j<terms_source_counter[i1]; j++)
    {
        if (f=="^" && terms[i1].sign == "-")
            term_vals[term_sources[i1][j]] = -oprt(f, -val1, val2);
        else
            term_vals[term_sources[i1][j]] = oprt(f, val1, val2);
    }
    terms_calculated[i1] = true;
    terms_calculated[i2] = true;
    int i=term_sources[i1][0];
    return term_vals[i];
}

int Expression::lookup_operators(const string &s)
{
    for (unsigned int i=0; i<operators.size(); i++)
        if (operators[i]==s)
            return i;
    return -1;

}

int Expression::count_operators(const string &s)
{
    int count = 0;
    for (unsigned int i=0; i<operators.size(); i++)
        if (operators[i]==s)
            count ++;
    return count;

}

vector<string> Expression::GetAllRequieredStartingBlockProperties()
{
    vector<string> s;
    if (param_constant_expression == "parameter")
    {
        if (location == loc::source)
            s.push_back(parameter);
        return s;
    }
    if (param_constant_expression == "expression")
    {
        for (unsigned int i = 0; i < terms.size(); i++)
        {
            for (unsigned int j = 0; j < terms[i].GetAllRequieredStartingBlockProperties().size(); j++)
                s.push_back(terms[i].GetAllRequieredStartingBlockProperties()[j]);
        }
        return s;
    }
    return s;
}
vector<string> Expression::GetAllRequieredEndingBlockProperties()
{
    vector<string> s;
    if (param_constant_expression == "parameter")
    {
        if (location == loc::destination)
            s.push_back(parameter);
        return s;
    }
    if (param_constant_expression == "expression")
    {
        for (unsigned int i = 0; i < terms.size(); i++)
        {
            for (unsigned int j = 0; j < terms[i].GetAllRequieredEndingBlockProperties().size(); j++)
                s.push_back(terms[i].GetAllRequieredEndingBlockProperties()[j]);
        }
        return s;
    }
    return s;
}

Expression Expression::ReviseConstituent(const string &constituent_name, const string &quantity)
{
    Expression out = *this;
    if (param_constant_expression=="parameter")
    {
        if (parameter==quantity)
        {   out.parameter = constituent_name + ":" + quantity;
            return out;
        }
    }
    for (unsigned int i=0; i<terms.size(); i++)
    {
        if (terms[i].param_constant_expression == "parameter")
        {
            if (out.terms[i].parameter == quantity)
            {
                out.terms[i].parameter = constituent_name + ":" + quantity;
            }

        }
        else if (terms[i].terms.size() > 0)
        {
            out.terms[i] = terms[i].ReviseConstituent(constituent_name,quantity);
        }
    }
    return out;
}

Expression Expression::RenameConstituent(const string &old_constituent_name, const string &new_constituent_name, const string &quantity)
{
    Expression out = *this;
    if (param_constant_expression=="parameter")
    {
        if (parameter==old_constituent_name + ":" + quantity)
        {   out.parameter = new_constituent_name + ":" + quantity;
            return out;
        }
    }
    for (unsigned int i=0; i<terms.size(); i++)
    {
        if (terms[i].param_constant_expression == "parameter")
        {
            if (out.terms[i].parameter == old_constituent_name + ":" + quantity)
            {
                out.terms[i].parameter = new_constituent_name + ":" + quantity;
            }

        }
        else if (terms[i].terms.size() > 0)
        {
            out.terms[i] = terms[i].RenameConstituent(old_constituent_name,new_constituent_name, quantity);
        }
    }
    return out;
}


bool Expression::RenameQuantity(const string &oldname, const string &newname)
{
    bool out=false;
    if (param_constant_expression=="parameter")
    {
        if (parameter==oldname)
        {   parameter = newname;
            return true;
        }
    }
    for (unsigned int i=0; i<terms.size(); i++)
    {
        if (terms[i].param_constant_expression == "parameter")
        {
            if (terms[i].parameter == oldname)
            {
                terms[i].parameter = newname;
                out = true;
            }

        }
        else if (terms[i].param_constant_expression == "expression")
        {
            out = out || terms[i].RenameQuantity(oldname,newname);
        }
    }
    return out;
}

string Expression::ToString() const
{
    string out;
    if (param_constant_expression=="parameter")
    {
        out += parameter;
        if (location == loc::source)
            out+=".s";
        if (location == loc::destination)
            out+=".e";
        if (location == loc::average_of_links)
            out+=".v";
        return out;
    }
    if (param_constant_expression=="constant")
    {
        out += aquiutils::numbertostring(constant);
        return out;
    }
    if (function!="") out += "_" + function;
    out += "(";
    for (unsigned int i=0; i<terms.size();i++)
    {
        out += terms[i].ToString();
        if (i<terms.size()-1) out += operators[i];
    }
    out += ")";
    return out;
}

void Expression::Setup_Calculation_Structure(){


    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "^")
        {
            _calculation_pattern pattern;
            pattern.operands.push_back(i);
            pattern.operands.push_back(i+1);
            pattern.output_cell_id = CalculationStructure.sources.size();
            CalculationStructure.CalcOrder.push_back(pattern);
            if (aquiutils::lookup(CalculationStructure.sources,i)==-1)
                CalculationStructure.sources.push_back(i);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);

            if (aquiutils::lookup(CalculationStructure.sources,i+1)==-1)
                CalculationStructure.sources.push_back(i+1);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
        }

    }
    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "*")
        {
            _calculation_pattern pattern;
            pattern.operands.push_back(i);
            pattern.operands.push_back(i+1);
            pattern.output_cell_id = CalculationStructure.CalcOrder.size();
            CalculationStructure.CalcOrder.push_back(pattern);
            if (aquiutils::lookup(CalculationStructure.sources,i)==-1)
                CalculationStructure.sources.push_back(i);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);

            if (aquiutils::lookup(CalculationStructure.sources,i+1)==-1)
                CalculationStructure.sources.push_back(i+1);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
        }
    }

    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "/")
        {
            _calculation_pattern pattern;
            pattern.operands.push_back(i);
            pattern.operands.push_back(i+1);
            pattern.output_cell_id = CalculationStructure.CalcOrder.size();
            CalculationStructure.CalcOrder.push_back(pattern);
            if (aquiutils::lookup(CalculationStructure.sources,i)==-1)
                CalculationStructure.sources.push_back(i);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);

            if (aquiutils::lookup(CalculationStructure.sources,i+1)==-1)
                CalculationStructure.sources.push_back(i+1);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
        }
    }

    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "+")
        {
            _calculation_pattern pattern;
            pattern.operands.push_back(i);
            pattern.operands.push_back(i+1);
            pattern.output_cell_id = CalculationStructure.CalcOrder.size();
            CalculationStructure.CalcOrder.push_back(pattern);
            if (aquiutils::lookup(CalculationStructure.sources,i)==-1)
                CalculationStructure.sources.push_back(i);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);

            if (aquiutils::lookup(CalculationStructure.sources,i+1)==-1)
                CalculationStructure.sources.push_back(i+1);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
        }
    }

    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "-")
        {
            _calculation_pattern pattern;
            pattern.operands.push_back(i);
            pattern.operands.push_back(i+1);
            pattern.output_cell_id = CalculationStructure.CalcOrder.size();
            CalculationStructure.CalcOrder.push_back(pattern);
            if (aquiutils::lookup(CalculationStructure.sources,i)==-1)
                CalculationStructure.sources.push_back(i);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);

            if (aquiutils::lookup(CalculationStructure.sources,i+1)==-1)
                CalculationStructure.sources.push_back(i+1);
            else
                CalculationStructure.sources.push_back(-1000-pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
            CalculationStructure.targets.push_back(pattern.output_cell_id);
        }
    }

}

