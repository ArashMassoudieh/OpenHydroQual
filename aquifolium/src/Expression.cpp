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

Expression::Expression(void)
{
    funcs.push_back("_min");
    funcs.push_back("_max");
    funcs.push_back("_exp");
    funcs.push_back("_log");
    funcs.push_back("_abs");
    funcs.push_back("_sgn");
    funcs.push_back("_sqr");
    funcs.push_back("_sqt");
    funcs.push_back("_pos");
    funcs.push_back("_hsd");
    funcs.push_back("_ups");
    funcs.push_back("_bkw");
    funcs.push_back("_mon");
    funcs.push_back("_mbs");
    opts.push_back("+");
    opts.push_back("-");
    opts.push_back("*");
    opts.push_back(";");
    opts.push_back("/");
    opts.push_back("^");

}

Expression::Expression(string S)
{
	text = S;
	#ifdef Debug_mode
	//cout<<text<<endl;
	#endif // Debug_mode

	funcs.push_back("_min");
	funcs.push_back("_max");
	funcs.push_back("_exp");
	funcs.push_back("_log");
	funcs.push_back("_abs");
	funcs.push_back("_sgn");
	funcs.push_back("_sqr");
    funcs.push_back("_sqt");
	funcs.push_back("_pos");
	funcs.push_back("_hsd");
	funcs.push_back("_ups");
    funcs.push_back("_bkw");
	funcs.push_back("_mon");
	funcs.push_back("_mbs");
	opts.push_back("+");
	opts.push_back("-");
	opts.push_back("*");
	opts.push_back(";");
	opts.push_back("/");
	opts.push_back("^");

	vector<string> out;
	//bool inside_quote = false;
	unsigned int paranthesis_level = 0;
    int last_operator_location = -1;
	if (!aquiutils::parantheses_balance(S))
	{
		_errors.push_back("Parentheses do not match in" + S);
		return;
	}
    if (aquiutils::lookup(funcs, aquiutils::left(S,4))!=-1 )
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
			if (aquiutils::lookup(funcs, aquiutils::left(S,4))!=-1)
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
            }
		}
	}


}

Expression::Expression(const Expression & S)
{
	operators = S.operators;
	constant = S.constant;
	terms = S.terms;
	sign = S.sign;
	funcs = S.funcs;
	opts = S.opts;
	function = S.function;
	parameter = S.parameter;
	param_constant_expression = S.param_constant_expression;
	unit = S.unit;
	text = S.text;
	location = S.location;
    term_sources.clear();
    term_sources_determined = false;

}

Expression & Expression::operator=(const Expression &S)
{
	operators = S.operators;
	constant = S.constant;
	terms = S.terms;
	sign = S.sign;
	funcs = S.funcs;
	opts = S.opts;
	function = S.function;
	parameter = S.parameter;
	param_constant_expression = S.param_constant_expression;
	unit = S.unit;
	text = S.text;
    location = S.location;
    term_sources.clear();
    term_sources_determined = false;

	return *this;
}


Expression::~Expression(void)
{
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

int aquiutils::lookup(const vector<string> &s, const string &s1)
{
    for (unsigned int i=0; i<s.size(); i++)
        if (s[i]==s1)
            return i;
    return -1;
}

int aquiutils::lookup(const vector<int> &s, const int &s1)
{
    for (unsigned int i=0; i<s.size(); i++)
        if (s[i]==s1)
            return i;
    return -1;
}

int aquiutils::lookup(const vector<vector<int> > &s, const vector<int> &s1)
{
    for (unsigned int i=0; i<s.size(); i++)
        if (s[i]==s1)
            return i;
    return -1;
}


void Expression::ResetTermsSources()
{
    if (!term_sources_determined)
    {
        term_sources.resize(terms.size());
        for (unsigned int j=0; j<terms.size(); j++)
            term_sources[j].resize(terms.size());
        terms_source_counter.resize(terms.size());
    }
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
        else
            quan = W->GetConnectedBlock(location)->Variable(parameter);
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

    if (!term_sources_determined)
    {   term_vals.resize(terms.size());
        terms_calculated.resize(terms.size());
    }

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
        else
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
            terms_source_counter.resize(terms.size());


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
                if (i==operators.size()-1)
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
		return log(val);
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

	#ifdef Debug_mode
	//cout<<i1<<","<<i2<<endl;
	#endif // Debug_mode

    for (unsigned int j = 0; j < terms_source_counter[i1]; j++)
    {
        if (term_sources.size() > i2)
            for (unsigned int k=0; k<terms_source_counter[i2]; k++)
                if (aquiutils::lookup(term_sources[term_sources[i2][k]],term_sources[i1][j])==-1)
                {   term_sources[term_sources[i2][k]][terms_source_counter[term_sources[i2][k]]]=term_sources[i1][j];
                    terms_source_counter[term_sources[i2][k]]++;
                }

    }
    if (term_sources.size() > i2)
        for (unsigned int j = 0; j < terms_source_counter[i2]; j++)
        {
            for (unsigned int k = 0; k<terms_source_counter[i1]; k++)
                if (aquiutils::lookup(term_sources[term_sources[i1][k]],term_sources[i2][j])==-1)
                {   term_sources[term_sources[i1][k]][terms_source_counter[term_sources[i1][k]]] = term_sources[i2][j];
                    terms_source_counter[term_sources[i1][k]]++;

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

    return term_vals[term_sources[i1][0]];
}

int aquiutils::corresponding_parenthesis(string S, int i)
{
	string s = S;
    if (S.at(i) == '(')
	{
		int paranthesis_level = 1;
		for (unsigned int j = i+1; j < S.size(); j++)
		{
            if (S.at(j) == '(')
				paranthesis_level++;
            if (S.at(j) == ')')
				paranthesis_level--;

			if (paranthesis_level == 0)
				return j;
		}
		return -1;
	}


    if (S.at(i) == ')')
	{
		int paranthesis_level = 1;
		for (int j = i-1; j > 0; j--)
		{
            if (S.at(j) == ')')
				paranthesis_level++;
            if (S.at(j) == '(')
				paranthesis_level--;

			if (paranthesis_level == 0)
				return j;
		}
		return -1;
	}
	return -1;
}

bool aquiutils::parantheses_balance(string S)
{
	if (count(S,"(") == count(S,")"))
		return true;
	else
		return false;
}

bool aquiutils::contains(const string &s, const string &s1)
{
    for (unsigned int i=0; i<s.size()-s1.size()+1; i++)
        if (s.substr(i,s1.size())==s1)
            return true;
    return false;
}

int aquiutils::count(const string &s, const string &s1)
{
    int out=0;
    for (unsigned int i=0; i<s.size()-s1.size()+1; i++)
        if (s.substr(i,s1.size())==s1)
            out++;
    return out;
}


string aquiutils::left(const string &s, int i)
{
    return s.substr(0,i);
}
string aquiutils::right(const string &s, int i)
{
    return s.substr(s.size()-i,i);
}

void aquiutils::remove(string &s,unsigned int i)
{
    string S;
    for (unsigned int j=0; j<s.size(); j++)
        if (i!=j)
            S = S + s[j];
    s = S;
}

void aquiutils::replace(string &s,unsigned int i,string p)
{
    string S;
    for (unsigned int j=0; j<s.size(); j++)
        if (i!=j)
            S = S + s[j];
        else
            S = S + p;
    s = S;
}

bool aquiutils::replace(string &s,string s1, string s2)
{

    bool found = false;
    vector<int> pos;
    unsigned int j=0;
    while (j<s.size()-s1.size()+1)
    {
        if (s.substr(j,s1.size())==s1)
        {
            pos.push_back(j);
            remove(s,j,s1.size());
            found = true;
        }
        j++;
    }
    for (unsigned int j=0; j<pos.size(); j++)
    {
        insert(s,pos[j]+j*s2.size(),s2);
    }

    return found;
}

bool aquiutils::remove(string &s, string s1)
{
    bool found = false;
    for (unsigned int j=0; j<s.size()-s1.size()+1; j++)
        if (s.substr(j,s1.size())==s1)
        {
            remove(s,j,s1.size());
            found = true;
        }
    return found;
}

void aquiutils::insert(string &s, unsigned int pos, string s1)
{
    string S;

    for (unsigned int i=0; i<s.size(); i++)
    {
        if (i==pos)
            S = S + s1;
        S = S + s[i];
    }
    if (pos==s.size()) S=S+s1;
    s = S;
}

void aquiutils::replace(string &s,unsigned int i, unsigned int j, string p)
{
    string S;
    for (unsigned int k=0; k<s.size(); k++)
        if (k==i)
            S = S + p;
        else if (k<i || k>=i+j)
            S = S + s[j];

    s = S;
}


void aquiutils::remove(string &s,unsigned int pos, unsigned int len)
{
    for (unsigned int i=pos; i<pos+len; i++)
        remove(s, pos);
}

bool aquiutils::isnumber(char S)
{
	if ((((int)S > 47) && ((int)S < 58)) || (S=='.'))
		return true;
	else
		return false;
}

bool aquiutils::isnumber(string S)
{
	bool res = true;
	for (unsigned int i = 0; i < S.size(); i++)
		if (!isnumber(S[i]))
			res = false;

	return res;
}


bool aquiutils::isintegernumber(string S)
{
	bool out = true;
	for (unsigned int i = 0; i < S.size(); i++)
	{
		if (((int)S[i] <= 47) || ((int)S[i] >= 58))
			out = false;
	}
	return out;
}

double aquiutils::atof(const string &S)
{
    return std::atof(S.c_str());
}
double aquiutils::atoi(const string &S)
{
    return std::atoi(S.c_str());
}

string aquiutils::trim(const string &s)
{
	if (s.find_first_not_of(' ') == string::npos) return "";

	return s.substr( s.find_first_not_of(' '), s.find_last_not_of(' ') + 1 );
}

void aquiutils::push_back(vector<string> &s, const vector<string> &s1)
{
    for (unsigned int i=0; i<s1.size(); i++)
        s.push_back(s1[i]);
}

bool aquiutils::isnegative(const double &x)
{
    if (x<-SMALLNUMBER)
        return true;
    else
        return false;
}

bool aquiutils::ispositive(const double &x)
{
    if (x>SMALLNUMBER)
        return true;
    else
        return false;
}

bool aquiutils::iszero(const double &x)
{
    if (fabs(x)<SMALLNUMBER)
        return true;
    else
        return false;
}



vector<string> aquiutils::split(const string &s, char del)
{
	unsigned int lastdel=0;
	vector<string> strings;
	for (unsigned int i=0; i<s.size(); i++)
	{
		if (s[i]==del)
		{
			strings.push_back(s.substr(lastdel, i-lastdel));
			lastdel = i+1;
		}
	}
    if (lastdel<s.size() && trim(s.substr(lastdel, s.size()-lastdel))!="\r" && trim(s.substr(lastdel, s.size() - lastdel)) != "") strings.push_back(trim(s.substr(lastdel, s.size()-lastdel)));  // works w/o trim- Trim can be deleted
	for (unsigned int i=0; i<strings.size(); i++) strings[i] = trim(strings[i]);					// Trim can be deleted
	if (strings.size()==1)
        if (strings[0]=="")
            strings.pop_back();
	return strings;

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

vector<string> aquiutils::getline(ifstream& file)
{
	string line;

	while (!file.eof())
	{
		std::getline(file, line);
		return split(line,',');
	}
	vector<string> x;
	return x;
}

vector<string> aquiutils::getline(ifstream& file, char del1)
{
    string line;

	while (!file.eof())
	{
		std::getline(file, line);
		return split(line,del1);
	}
	vector<string> x;
	return x;
}

vector<vector<string>> aquiutils::getline_op(ifstream& file,char del1)
{
	string line;
	vector<vector<string>> s;
	vector<string> ss;
	while (file.good())
	{
		getline(file, line);
		ss = split(line,',');
		for (unsigned int i=0; i<ss.size(); i++)
			s.push_back(split(ss[i],del1));
	}
	return s;

}

vector<vector<string>> aquiutils::getline_op(ifstream& file,vector<char> del1)
{
		string line;
	vector<vector<string>> s;
	vector<string> ss;
	while (file.good())
	{
		getline(file, line);
		ss = split(line,',');
		for (unsigned int i=0; i<ss.size(); i++)
			s.push_back(split(ss[i],del1));
	}
	return s;
}

vector<vector<string>> aquiutils::getline_op_eqplus(ifstream& file)
{
	vector<char> del1;
	del1.push_back('=');
	del1.push_back('+');
	string line;
	vector<vector<string>> s;
	vector<string> ss;
	while (file.good())
	{
		getline(file, line);
		ss = split(line,',');
		for (unsigned int i=0; i<ss.size(); i++)
			s.push_back(split(ss[i],del1));
	}
	return s;


}

vector<string> aquiutils::split(const string &s, const vector<char> &del)
{
	unsigned int lastdel=0;
	unsigned int j=0;
	vector<string> strings;
	for (unsigned int i=0; i<s.size(); i++)
	{
		for (unsigned int jj=0; jj<del.size(); jj++)
		if (s[i]==del[jj])
		{
			strings.push_back(s.substr(lastdel, i-lastdel));
			lastdel = i+1;
			j++;
		}
	}
	if (lastdel<s.size()) strings.push_back(trim(s.substr(lastdel, s.size()-lastdel)));
	for (unsigned int i=0; i<strings.size(); i++) strings[i] = trim(strings[i]);
	return strings;

}

vector<string> aquiutils::split_curly_semicolon(string s)
{
	vector<char> del2; del2.push_back('{'); del2.push_back('}'); del2.push_back(';');
	return split(s,del2);
}

vector<int> aquiutils::look_up(string s, char del)  //Returns a vector with indices of "del"
{
	vector<int> out;
	for (unsigned int i=0; i<s.size(); i++)
		if (s[i]==del)
			out.push_back(i);

	return out;

}

vector<int> aquiutils::ATOI(vector<string> ii)
{
	vector<int> res;
	for (unsigned int i=0; i<ii.size(); i++)
		res.push_back(atoi(ii[i].c_str()));

	return res;
}

vector<double> aquiutils::ATOF(vector<string> ii)
{
	vector<double> res;
	for (unsigned int i=0; i<ii.size(); i++)
		res.push_back(atof(ii[i].c_str()));

	return res;
}


string aquiutils::tolower(const string &S)
{
	string SS = S;
	for (unsigned int i=0; i<S.size(); i++)
	{
		SS[i] = std::tolower(S[i]);
	}
	return SS;
}

vector<string> aquiutils::tolower(const vector<string> &S)
{
	vector<string> SS = S;
	for (unsigned int i = 0; i<S.size(); i++)
	{
		SS[i] = tolower(S[i]);
	}
	return SS;
}

void aquiutils::writeline(ofstream& f, vector<string> s, string del=",")
{
	for (unsigned int i=0; i<s.size()-1; i++)
		f<<s[i]<<del;
	f<<s[s.size()-1]<<std::endl;
}

void aquiutils::writeline(ofstream& f, vector<vector<string>> s, string del=",", string del2="&")
{
	for (unsigned int i=0; i<s.size()-1; i++)
	{	for (unsigned int j=0; j<s[i].size()-1; j++)
			f<<s[i][j]<<del2;
		f<<s[i][s[i].size()-1]<<del;
	}
	f<<s[s.size()-1][s[s.size()-1].size()-1]<<std::endl;
}
void aquiutils::writestring(ofstream& f, string s)
{
	f<<s;
}

void aquiutils::writestring(string filename, string s)
{
	ofstream file(filename);
	file << s + "\n";
	file.close();

}
void aquiutils::writenumber(ofstream& f, double s)
{
	f<<s;
}

void aquiutils::writeendl(ofstream& f)
{
	f<<std::endl;
}

double aquiutils::Heavyside(double x)
{
	if (x>0) return 1; else return 0;
}

double aquiutils::Pos(double x)
{
	if (x>0) return x; else return 0;
}

string aquiutils::numbertostring(double x, bool scientific)
{
	string Result;          // string which will contain the result
	ostringstream convert;   // stream used for the conversion
    if (scientific)
        convert << std::scientific;
    convert << x;      // insert the textual representation of 'Number' in the characters in the stream
	Result = convert.str();
	return Result;
}

string aquiutils::numbertostring(vector<double> x, bool scientific)
{
	string Result = "[";
    for (unsigned int i=0; i<x.size()-1;i++)
        Result += aquiutils::numbertostring(x[i],scientific)+",";
    Result += aquiutils::numbertostring(x[x.size()-1],scientific) + "]";
	return Result;
}

string aquiutils::numbertostring(int x)
{
	string Result;          // string which will contain the result
	ostringstream convert;   // stream used for the conversion
	convert << x;      // insert the textual representation of 'Number' in the characters in the stream
	Result = convert.str();
	return Result;
}

string aquiutils::numbertostring(unsigned int x)
{
    string Result;          // string which will contain the result
    ostringstream convert;   // stream used for the conversion
    convert << x;      // insert the textual representation of 'Number' in the characters in the stream
    Result = convert.str();
    return Result;
}

string aquiutils::numbertostring(vector<int> x)
{
	string Result = "[";
	if (x.size()>0)
    {   for (unsigned int i=0; i<x.size()-1;i++)
           Result += aquiutils::numbertostring(x[i])+",";
        Result += aquiutils::numbertostring(x[x.size()-1]) + "]";
    }
    else
        Result += "]";
	return Result;
}

string aquiutils::tail(std::string const& source, size_t const length) {
	if (length >= source.size()) { return source; }
	return source.substr(source.size() - length);
} // tail

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
        return out;
    }
    if (param_constant_expression=="constant")
    {
        out += aquiutils::numbertostring(constant);
        return out;
    }
    if (function!="") out += function;
    out += "(";
    for (unsigned int i=0; i<terms.size();i++)
    {
        out += terms[i].ToString();
        if (i<terms.size()-1) out += operators[i];
    }
    out += ")";
    return out;
}

string aquiutils::tabs(int i)
{
    string out;
    for (int j=0; j<i; j++)
        out += "\t";
    return out;
}

bool aquiutils::And(vector<bool> x) { bool out = true;  for (unsigned int i = 0; i < x.size(); i++) out &= x[i]; return out; }
double aquiutils::Max(vector<double> x) { double out = -1e+24;  for (unsigned int i = 0; i < x.size(); i++) out=std::max(out, x[i]); return out; }
int aquiutils::Max(vector<int> x)
{	int out = -37000;
    for (unsigned int i = 0; i < x.size(); i++)
		out=std::max(out, x[i]);
	return out;

}

string aquiutils::remove_backslash_r(const string &ss)
{
    string s = ss;
    if (!s.empty() && s[s.size() - 1] == '\r')
        s.erase(s.size() - 1);
    return s;

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

string aquiutils::GetOnlyFileName(const string &fullfilename)
{
    vector<char> del;
    del.push_back('/');
    del.push_back('\\');
    vector<string> splittedbyslash = aquiutils::split(fullfilename,del);
    return splittedbyslash[splittedbyslash.size()-1];

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

