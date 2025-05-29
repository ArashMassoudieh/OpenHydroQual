#include "Wizard_Argument.h"
#include "Utilities.h"
#include <QDateTime>
#include <QVector>
#include <QString>
#include "weatherretriever.h"
#include "Wizard_Entity.h"
#include "Wizard_Script.h"

bool Wizard_Argument::func_operators_initialized =false;
vector<string> Wizard_Argument::funcs = vector<string>();
vector<string> Wizard_Argument::opts = vector<string>();


Wizard_Argument::Wizard_Argument()
{
    if (Wizard_Argument::func_operators_initialized != true)
    {
        Wizard_Argument::funcs.push_back("_min");
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

}
Wizard_Argument::Wizard_Argument(const string& _S, const string& _unit)
{
    if (Wizard_Argument::func_operators_initialized != true)
    {
        Wizard_Argument::funcs.push_back("_min");
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
    string S = _S; 
    text = S;
    
    if (QString::fromStdString(S).contains("@sp"))
    {
        argument_type = parameter_type::string;
        parameter = QString::fromStdString(S).split("@")[0].toStdString();
        param_constant_expression = "parameter"; 
        return; 
    }
    else if (QString::fromStdString(S).contains("@sc"))
    {
        argument_type = parameter_type::string;
        constant_string = QString::fromStdString(S).split("@")[0].toStdString();
        param_constant_expression = "constant";
        return;
    }
    if (QString::fromStdString(S).contains("@dp"))
    {
        argument_type = parameter_type::date;
        parameter = QString::fromStdString(S).split("@")[0].toStdString();
        param_constant_expression = "parameter";
        return;
    }
    else if (QString::fromStdString(S).contains("@dc"))
    {
        argument_type = parameter_type::date;
        constant = QDate2Xldate(QDateTime::fromString(QString::fromStdString(S).split("@")[0], "MM.dd.yyyy"));
        param_constant_expression = "constant";
        return;
    }
    else if (QString::fromStdString(S).contains("@api"))
    {
        argument_type = parameter_type::api;
        parameter = QString::fromStdString(S).split("@")[0].toStdString();
        param_constant_expression = "api";
        return;
    }

    
    unit = _unit; 
    
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
            param_constant_expression = "parameter";
            parameter = S; 
        }
    }


}
Wizard_Argument::Wizard_Argument(const Wizard_Argument& WA)
{
    operators = WA.operators;
    terms = WA.terms;
    text = WA.text;
    funcs = WA.funcs;
    opts = WA.opts;
    func_operators_initialized = WA.func_operators_initialized;
    _errors = WA._errors;
    param_constant_expression = WA.param_constant_expression;
    constant = WA.constant;
    function = WA.function;
    parameter = WA.parameter;
    sign = WA.sign;
    CalculationStructure = WA.CalculationStructure;
    unit = WA.unit;
    argument_type = WA.argument_type;
    constant_string = WA.constant_string;
    wizard_entity = WA.wizard_entity;
}
Wizard_Argument& Wizard_Argument::operator=(const Wizard_Argument& WA)
{
    operators = WA.operators;
    terms = WA.terms;
    text = WA.text;
    funcs = WA.funcs;
    opts = WA.opts;
    func_operators_initialized = WA.func_operators_initialized;
    _errors = WA._errors;
    param_constant_expression = WA.param_constant_expression;
    constant = WA.constant;
    function = WA.function;
    parameter = WA.parameter;
    sign = WA.sign;
    CalculationStructure = WA.CalculationStructure;
    unit = WA.unit;
    argument_type = WA.argument_type;
    constant_string = WA.constant_string;
    wizard_entity = WA.wizard_entity;
    return *this;
}

void Wizard_Argument::Setup_Calculation_Structure() {


    CalculationStructure.CalcOrder.clear();
    CalculationStructure.targets.clear();
    CalculationStructure.sources.clear();
    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "^")
            AppendTermToStructure(i);


    }
    for (int i = 0; i < operators.size(); i++)
    {
        if (operators[i] == "/")
        {
            AppendTermToStructure(i);
        }
    }

    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "*")
        {
            AppendTermToStructure(i);
        }
    }

    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "+")
        {
            AppendTermToStructure(i);
        }
    }

    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == "-")
        {
            AppendTermToStructure(i);
        }
    }
    for (int i = operators.size() - 1; i >= 0; i--)
    {
        if (operators[i] == ";")
        {
            AppendTermToStructure(i);
        }
    }
    if (operators.empty())
    {
        _calculation_pattern pattern;
        pattern.operands.push_back(0);
        pattern.output_cell_id = 0;
        CalculationStructure.CalcOrder.push_back(pattern);
        CalculationStructure.sources.push_back(0);
        CalculationStructure.targets.push_back(0);
    }

}

void Wizard_Argument::AppendTermToStructure(int i)
{
    _calculation_pattern pattern;
    pattern.operands.push_back(i);
    pattern.operands.push_back(i + 1);
    pattern.operatr = operators[i];
    if (i > 0)
        pattern.sign = operators[i - 1];
    pattern.output_cell_id = CalculationStructure.CalcOrder.size();
    int order_of_source_container1 = -1;
    int order_of_source_container2 = -1;
    CalculationStructure.CalcOrder.push_back(pattern);
    if (aquiutils::lookup(CalculationStructure.sources, i, true) != -1)
    {
        order_of_source_container1 = get_target_item_of_term(i);
    }

    if (aquiutils::lookup(CalculationStructure.sources, i + 1, true) != -1)
    {
        order_of_source_container2 = get_target_item_of_term(i + 1);

    }
    if (order_of_source_container1 != -1)
        CalculationStructure.sources.push_back(-1000 - order_of_source_container1);
    else
        CalculationStructure.sources.push_back(i);
    if (order_of_source_container2 != -1)
        CalculationStructure.sources.push_back(-1000 - order_of_source_container2);
    else
        CalculationStructure.sources.push_back(i + 1);
    CalculationStructure.targets.push_back(pattern.output_cell_id);
    CalculationStructure.targets.push_back(pattern.output_cell_id);
}

int Wizard_Argument::get_target_item_of_term(int term_id)
{
    int out;
    if (aquiutils::lookup(CalculationStructure.sources, term_id, true) == -1)
        return -1;
    else
    {
        out = CalculationStructure.targets[aquiutils::lookup(CalculationStructure.sources, term_id, true)];
        if (get_target_item_of_term(-1000 - out) != -1)
            out = get_target_item_of_term(-1000 - out);
    }
    return out;
}

double Wizard_Argument::calc(QMap<QString,WizardParameter> *params)
{
   
    if (param_constant_expression == "constant")
        return constant;
    if (param_constant_expression == "parameter")
    {
        double out = 0;
        out = params->operator[](QString::fromStdString(parameter)).Value().toDouble();
        

        if (function.empty())
            return out;
        else if (count_operators(";") == 0)
            return func(function, out);
    }
    if (param_constant_expression == "expression")
    {
        if (operators.empty())
        {
            CalculationStructure.CalcOrder[0].value = terms[0].calc(params);
        }
        else
        {
            for (unsigned int i = 0; i < CalculationStructure.CalcOrder.size(); i++)
            {
                oprt(i, CalculationStructure.CalcOrder[i].operatr, params);
            }
        }
        if (function.empty())
            return CalculationStructure.CalcOrder[CalculationStructure.CalcOrder.size() - 1].value;
        else if (count_operators(";") == 0)
            return func(function, CalculationStructure.CalcOrder[CalculationStructure.CalcOrder.size() - 1].value);
        else if (count_operators(";") == 1)
        {
            vector<double> vals = argument_values(CalculationStructure.CalcOrder.size() - 1, params);
            return func(function, vals[0], vals[1]);
        }
        else if (count_operators(";") == 2)
        {
            vector<double> vals1 = argument_values(CalculationStructure.CalcOrder.size() - 1, params);
            vector<double> vals2 = argument_values(CalculationStructure.CalcOrder.size() - 2, params);
            return func(function, vals1[0], vals1[1], vals2[0]);
        }
    }

    return 0;
}

int  Wizard_Argument::count_operators(const string& s) const
{
    int count = 0;
    for (unsigned int i = 0; i < operators.size(); i++)
    {
        if (operators[i] == s)
            count++;
    }
    return count;

}

QString Wizard_Argument::Calc(QMap<QString, WizardParameter>* params)
{
    if (param_constant_expression == "api")
    {
        WeatherRetriever weatherretriever;
        //weatherretriever.SetAPIToken("AuOQEjHeTwRMJeUjLpoXmneFKxUDdred");
        QStringList delegate = params->value(QString::fromStdString(parameter)).Delegate().split("|");
        qDebug()<<delegate[1]<<":"<<params->value(delegate[1]).Value();
        double x_location = params->value(delegate[1]).Value().toDouble();
        double y_location = params->value(delegate[2]).Value().toDouble();
        double start_date = params->value(delegate[3]).Value().toDouble();
        double end_date = params->value(delegate[4]).Value().toDouble();
        QPointF location(y_location, x_location);
        QString FileName;
        if (parameter == "PrecipitationData")
        {   CPrecipitation precipitationdata = weatherretriever.RetrivePrecipOpenMeteo(start_date, end_date, location);
            wizard_entity->GetWizardScript()->AppendTimeSeries(this->wizard_entity->Name(),this->wizard_entity->Name() + QString::number(x_location) + "_" + QString::number(y_location) + ".csv");
            FileName = WorkingDirectory() + "/" + this->wizard_entity->Name() + QString::number(x_location) + "_" + QString::number(y_location) + ".csv";
            precipitationdata.writefile(FileName.toStdString());
        }
        else if (parameter == "TemperatureData")
        {
            CTimeSeries<double> temperaturedata = weatherretriever.RetriveTimeSeriesOpenMeteo(start_date, end_date, location, "temperature_2m");
            wizard_entity->GetWizardScript()->AppendTimeSeries(this->wizard_entity->Name(),this->wizard_entity->Name() + QString::number(x_location) + "_" + QString::number(y_location) + ".csv");
            FileName = WorkingDirectory() + "/" + this->wizard_entity->Name() + QString::number(x_location) + "_" + QString::number(y_location) + ".csv";
            temperaturedata.writefile(FileName.toStdString());
        }

        return FileName;
    }
    if (param_constant_expression == "constant")
    {
        if (argument_type == parameter_type::string)
            return QString::fromStdString(constant_string);
        else
            return QString::number(constant);
    }
    if (param_constant_expression == "parameter")
    {
        if (params->value(QString::fromStdString(parameter)).ParameterType() == parameter_type::string)
        {
            return params->value(QString::fromStdString(parameter)).Value();
        }
    }
    return QString::number(calc(params));
}



double Wizard_Argument::func(const string& f, const double& val)
{

    if (f == "exp")
        return exp(val);
    if (f == "log")
    {
        if (val > 0)
            return log(val);
        else
            return -1e12;
    }
    if (f == "abs")
        return fabs(val);
    if (f == "sgn")
        return (val > 0 ? +1 : -1);
    if (f == "sqr")
        return sqrt(aquiutils::Pos(val));
    if (f == "sqt")
    {
        if (val > 0)
            return sqrt(val * val / (fabs(val) + 1e-4));
        else
            return -sqrt(val * val / (fabs(val) + 1e-4));
    }
    if (f == "pos")
        return (val + fabs(val)) / 2.0;
    if (f == "hsd")
    {
        if (val >= 0) return 1; else return 0;
    }
    return val;
}

double Wizard_Argument::func(const string& f, const double& val1, const double& val2)
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
        return pow(fabs(val1), 1 - (1 - val2) * val1 / (1e-6 + val1));
    return val1;
}

double Wizard_Argument::func(const string& f, const double& cond, const double& val1, const double& val2)
{
    if (f == "ups")
    {
        if (cond >= 0)
            return val1;
        else
            return val2;
    }
    else if (f == "bkw")
    {
        if (cond >= 0)
            return val1;
        else
            return val2;
    }

    return val1;
}


double Wizard_Argument::oprt(unsigned int calculation_sequence, string& f, QMap<QString, WizardParameter>* params)
{

    int i1 = CalculationStructure.sources[static_cast<std::vector<int, std::allocator<int>>::size_type>(calculation_sequence) * 2];
    int i2 = CalculationStructure.sources[static_cast<std::vector<int, std::allocator<int>>::size_type>(calculation_sequence) * 2 + 1];
    double val1 = 0;
    double val2 = 0;
    if (i1 >= 0 && i2 >= 0)
    {
        val1 = terms[CalculationStructure.CalcOrder[calculation_sequence].operands[0]].calc(params);
        val2 = terms[CalculationStructure.CalcOrder[calculation_sequence].operands[1]].calc(params);
        if (terms[CalculationStructure.CalcOrder[calculation_sequence].operands[0]].sign == "-")
            val1 = -val1;
        if (terms[CalculationStructure.CalcOrder[calculation_sequence].operands[1]].sign == "-")
            val2 = -val2;
    }
    else if (i1 < 0 && i2 >= 0)
    {
        val1 = CalculationStructure.CalcOrder[-i1 - 1000].value;
        val2 = terms[CalculationStructure.CalcOrder[calculation_sequence].operands[1]].calc(params);
        if (terms[CalculationStructure.CalcOrder[calculation_sequence].operands[1]].sign == "-")
            val2 = -val2;

    }
    else if (i2 < 0 && i1 >= 0)
    {
        val2 = CalculationStructure.CalcOrder[-i2 - 1000].value;
        val1 = terms[CalculationStructure.CalcOrder[calculation_sequence].operands[0]].calc(params);
        if (terms[CalculationStructure.CalcOrder[calculation_sequence].operands[0]].sign == "-")
            val1 = -val1;

    }
    else if (i2 < 0 && i1 < 0)
    {
        val2 = CalculationStructure.CalcOrder[-i2 - 1000].value;
        val1 = CalculationStructure.CalcOrder[-i1 - 1000].value;
    }

    if (f == "^" && terms[i1].sign == "-")
        CalculationStructure.CalcOrder[calculation_sequence].value = -oprt(f, -val1, val2);
    else
        CalculationStructure.CalcOrder[calculation_sequence].value = oprt(f, val1, val2);



    //qDebug()<<CalculationStructure.CalcOrder[calculation_sequence].value;

    return CalculationStructure.CalcOrder[calculation_sequence].value;

}

double Wizard_Argument::oprt(const string& f, const double& val1, const double& val2) const
{
    if (f == "^")
        return pow(aquiutils::Pos(val1), val2);
    if (f == "+") return
        val1 + val2;
    if (f == "-") return
        val1 + val2;
    if (f == "/") return
        val1 / (val2 + 1e-23);
    if (f == "*") return
        val1 * val2;
    return 0;
}

vector<double>  Wizard_Argument::argument_values(unsigned int calculation_sequence, QMap<QString, WizardParameter>* params)
{
    vector<double> out(2);
    const int i1 = CalculationStructure.sources[static_cast<std::vector<int, std::allocator<int>>::size_type>(calculation_sequence) * 2];
    const int i2 = CalculationStructure.sources[static_cast<std::vector<int, std::allocator<int>>::size_type>(calculation_sequence) * 2 + 1];
    double val1 = 0;
    double val2 = 0;
    if (i1 >= 0 && i2 >= 0)
    {
        val1 = terms[CalculationStructure.CalcOrder[calculation_sequence].operands[0]].calc(params);
        val2 = terms[CalculationStructure.CalcOrder[calculation_sequence].operands[1]].calc(params);
    }
    else if (i1 < 0 && i2 >= 0)
    {
        val1 = CalculationStructure.CalcOrder[-i1 - 1000].value;
        val2 = terms[CalculationStructure.CalcOrder[calculation_sequence].operands[1]].calc(params);
    }
    else if (i2 < 0 && i1 >= 0)
    {
        val2 = CalculationStructure.CalcOrder[-i2 - 1000].value;
        val1 = terms[CalculationStructure.CalcOrder[calculation_sequence].operands[0]].calc(params);
    }
    out[0] = val1;
    out[1] = val2;
    return out;
}

double QDate2Xldate(const QDateTime& x)
{
    QDateTime base_time1 = QDateTime::fromString("1-1-1900 00:00", "M-d-yyyy hh:mm");
    double xxx = (x.toMSecsSinceEpoch() - base_time1.toMSecsSinceEpoch()) / (1000.00 * 24.0 * 60.0 * 60.0) + 2;
    return xxx;
}

double QString2Xldate(const QString& x)
{
    QDateTime time = QDateTime::fromString(x, "MM.dd.yyyy");
    return QDate2Xldate(time);
}

QString Wizard_Argument::UnitText()
{
    if (unit=="")
        return "";
    else
        return QString::fromStdString("[" + unit + "]");
}

QString Wizard_Argument::WorkingDirectory()
{
    return wizard_entity->GetWizardScript()->WorkingFolder();
}
