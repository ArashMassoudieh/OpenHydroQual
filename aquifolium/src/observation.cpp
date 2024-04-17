#include "observation.h"
#include "BTC.h"
#include "System.h"


Observation::Observation(): Object::Object()
{
    SetObjectType(object_type::observation);
    //ctor
}

Observation::Observation(System *_system)
{
    SetObjectType(object_type::observation);
    system = _system;
}

Observation::Observation(System *_system, const Expression &expr, const string &loc)
{
    SetType("Observation");
    SetObjectType(object_type::observation);
    system = _system;
    expression = expr;
    location = loc;

}

Observation::~Observation()
{
    modeled_time_series.clear();
    observed_time_series.clear(); 
}

Observation::Observation(const Observation& other)
{
    Object::operator=(other);
    expression = other.expression;
    location = other.location;
    modeled_time_series = other.modeled_time_series;
    observed_time_series = other.observed_time_series;
}

Observation& Observation::operator=(const Observation& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    Object::operator=(rhs);
    expression = rhs.expression;
    location = rhs.location;
    observed_time_series = rhs.observed_time_series;
    modeled_time_series.clear();
    return *this;
}

bool Observation::SetProperty(const string &prop, const string &val)
{
    if (aquiutils::tolower(prop)=="expression")
    {
        expression = Expression(val);

    }
    if (aquiutils::tolower(prop)=="location" || aquiutils::tolower(prop)=="object")
    {
        location = val;

    }

    return Object::SetProperty(prop,val);
    return false;
}

double Observation::GetValue(const Expression::timing &tmg)
{
    if (expression.param_constant_expression == "")
        expression = Variable("expression")->GetProperty();

    if (system->block(Variable("object")->GetProperty()) != nullptr)
    {
        current_value = expression.calc(system->block(location),tmg);
        return current_value;
    }
    if (system->link(Variable("object")->GetProperty()) != nullptr)
    {
        current_value = expression.calc(system->link(location),tmg,true);
        return current_value;
    }
    lasterror = "Location " + location + "was not found in the system!";
    return 0;
}

double Observation::CalcMisfit()
{
    //qDebug()<<"Inside the misfit function";
    if (Variable("observed_data")->GetTimeSeries()!=nullptr)
    {

        fit_measures.clear();
        if (Variable("comparison_method")->GetProperty()=="Least Squared")
        {
            double fit_mse = 0;
            double _R2 = 0;
            double Nash_Sutcliffe_efficiency = 0;
            if (Variable("error_structure")->GetProperty()=="normal")
            {
                fit_mse = diff2(modeled_time_series,Variable("observed_data")->GetTimeSeries());
                _R2 = R2(&modeled_time_series,Variable("observed_data")->GetTimeSeries());
                Nash_Sutcliffe_efficiency = NSE(&modeled_time_series,Variable("observed_data")->GetTimeSeries());
                fit_measures.push_back(fit_mse);
                fit_measures.push_back(_R2);
                fit_measures.push_back(Nash_Sutcliffe_efficiency);
                return Variable("observed_data")->GetTimeSeries()->n*(fit_mse/pow(Variable("error_standard_deviation")->GetVal(),2)+log(Variable("error_standard_deviation")->GetVal()));

            }
            else if (Variable("error_structure")->GetProperty()=="log-normal" || Variable("error_structure")->GetProperty()=="lognormal")
            {
                fit_mse = diff2(modeled_time_series.Log(1e-8),Variable("observed_data")->GetTimeSeries()->Log(1e-8));
                _R2 = R2(modeled_time_series.Log(1e-8),Variable("observed_data")->GetTimeSeries()->Log(1e-8));
                Nash_Sutcliffe_efficiency = NSE(modeled_time_series.Log(1e-8),Variable("observed_data")->GetTimeSeries()->Log(1e-8));
                fit_measures.push_back(fit_mse);
                fit_measures.push_back(_R2);
                fit_measures.push_back(Nash_Sutcliffe_efficiency);
                return Variable("observed_data")->GetTimeSeries()->n*(fit_mse/pow(Variable("error_standard_deviation")->GetVal(),2)+log(Variable("error_standard_deviation")->GetVal()));
            }
            else
                return 0;
        }
        else
        {
            double auto_correlation_diff = 0;
            double CDF_diff = 0;
            double time_span = Variable("autocorrelation_time-span")->GetVal();
            double increment = time_span/20.0;
            CTimeSeries<double> autocorr_measured = Variable("observed_data")->GetTimeSeries()->ConverttoNormalScore().AutoCorrelation(time_span,increment);
            CTimeSeries<double> autocorr_modeled = modeled_time_series.ConverttoNormalScore().AutoCorrelation(time_span,increment);
            auto_correlation_diff =  diff2(autocorr_measured, autocorr_modeled);
            if (Variable("error_structure")->GetProperty()=="normal")
            {

                //qDebug()<<"Calculating Misfit Normal";
                CDF_diff = KolmogorovSmirnov(Variable("observed_data")->GetTimeSeries(),&modeled_time_series);

            }
            else
            {
                //qDebug()<<"Calculating Misfit Log-Normal";
                CDF_diff = KolmogorovSmirnov(Variable("observed_data")->GetTimeSeries()->Log(1e-8),modeled_time_series.Log(1e-8));
                //qDebug()<<"CDF diff calculated";
            }
            fit_measures.push_back(auto_correlation_diff + CDF_diff);
            fit_measures.push_back(auto_correlation_diff);
            fit_measures.push_back(CDF_diff);
            //qDebug()<<"Misfit vector populated";
            return auto_correlation_diff + CDF_diff;
        }
    }
    else
    {
        fit_measures.resize(3);
        return 0;
    }

}

void Observation::append_value(double t, double val)
{
    modeled_time_series.append(t,val);
    return;
}

void Observation::append_value(double t)
{
    current_value = GetValue(Expression::timing::present);
    modeled_time_series.append(t,current_value);
    return;
}

vector<string> Observation::ItemswithOutput()
{
    vector<string> s = Object::ItemswithOutput();
    s.push_back("Time Series");
    return s;
}






