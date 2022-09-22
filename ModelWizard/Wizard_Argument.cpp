#include "Wizard_Argument.h"
#include "Utilities.h"

bool Wizard_Argument::func_operators_initialized =false;
vector<string> Wizard_Argument::funcs = vector<string>();
vector<string> Wizard_Argument::opts = vector<string>();


Wizard_Argument::Wizard_Argument()
{

}
Wizard_Argument::Wizard_Argument(const string& S)
{
    text = S;

    if (Wizard_Argument::func_operators_initialized != true)
    {   Wizard_Argument::funcs.push_back("_min");
        Wizard_Argument::funcs.push_back("_max");
        Wizard_Argument::funcs.push_back("_exp");
        Wizard_Argument::funcs.push_back("_log");
        Wizard_Argument::funcs.push_back("_abs");
        Wizard_Argument::funcs.push_back("_sgn");
        Wizard_Argument::funcs.push_back("_sqr");
        Wizard_Argument::funcs.push_back("_sqt");
        Wizard_Argument::funcs.push_back("_lpw");
        Wizard_Argument::funcs.push_back("_pos");
        Wizard_Argument::funcs.push_back("_hsd");
        Wizard_Argument::funcs.push_back("_ups");
        Wizard_Argument::funcs.push_back("_bkw");
        Wizard_Argument::funcs.push_back("_mon");
        Wizard_Argument::funcs.push_back("_mbs");
        Wizard_Argument::funcs.push_back("_ekr");
        Wizard_Argument::funcs.push_back("_gkr");
        Wizard_Argument::opts.push_back("+");
        Wizard_Argument::opts.push_back("-");
        Wizard_Argument::opts.push_back("*");
        Wizard_Argument::opts.push_back(";");
        Wizard_Argument::opts.push_back("/");
        Wizard_Argument::opts.push_back("^");
        Wizard_Argument::func_operators_initialized = true;
    }


    QVector<QString> out;
    //bool inside_quote = false;
    int last_operator_location = -1;
    if (!aquiutils::parantheses_balance(S))
    {
        _errors.push_back("Parentheses do not match in" + S);
        return;
    }
    if (aquiutils::lookup(Wizard_Argument::funcs, aquiutils::left(S,4))!=-1 )
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
            //	terms.append(CWizard_Argument("0"));
            if (aquiutils::lookup(Wizard_Argument::funcs, aquiutils::left(S,4))!=-1)
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
            unsigned int parenthesis_level = 0;
            param_constant_expression = "expression";
            for (unsigned int i = 0; i < S.size(); i++)
            {
                if (S.substr(i, 1) == "(")
                    parenthesis_level++;

                if (S.substr(i, 1) == ")")
                    parenthesis_level--;

                if (parenthesis_level == 0)
                    if ((S.substr(i, 1) == "+") || (S.substr(i, 1) == "-") || (S.substr(i, 1) == "*") || (S.substr(i, 1) == "/") || (S.substr(i, 1) == "^") || (S.substr(i, 1) == ";"))
                    {
                        operators.push_back(S.substr(i, 1));
                        Wizard_Argument sub_exp = Wizard_Argument(aquiutils::trim(S.substr(last_operator_location+1, i -1- last_operator_location)));
                        if (!sub_exp.text.empty())
                        {
                            if (operators.size() > 1)
                                sub_exp.sign = operators[operators.size() - 2];
                            else
                                sub_exp.sign = "+";
                            terms.push_back(sub_exp);
                        }
                        else
                        {
                            sub_exp = Wizard_Argument("0");
                            sub_exp.sign = "+";
                            terms.push_back(sub_exp);
                        }
                        aquiutils::push_back(_errors,sub_exp._errors);
                        last_operator_location = i;
                    }
            }


            Wizard_Argument sub_exp = Wizard_Argument(aquiutils::trim(S.substr(last_operator_location+1, S.size() - last_operator_location)));
            if (!operators.empty())
                sub_exp.sign = operators[operators.size() - 1];
            else
                sub_exp.sign = "+";
            terms.push_back(sub_exp);
            aquiutils::push_back(_errors,sub_exp._errors);
            Setup_Calculation_Structure();

        }
        else
        {
            param_constant_Wizard_Argument = "parameter";
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
Wizard_Argument::Wizard_Argument(const Wizard_Argument& WA)
{

}
Wizard_Argument& Wizard_Argument::operator=(const Wizard_Argument& WA)
{
	return *this;
}
