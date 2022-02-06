#ifndef EXPRESSION_H
#define EXPRESSION_H
#include <string>
#include <vector>
#include <algorithm>
#include "Utilities.h"
using namespace std;

class Object;
class Quan;

class Expression
{
    public:
        virtual ~Expression();
        Expression(void);
        Expression(string S);
        Expression(const Expression &S);
        Expression& operator=(const Expression&);
        vector<string> operators;
        vector<Expression> terms;
        Quan* quan = nullptr;
        string sign;
        string function;
        string parameter;
        double constant;
        string param_constant_expression;
        vector<string> extract_operators(string s);
        vector<string> _errors;
        vector<string> extract_terms(string s);
        enum class timing {past, present, both};
        double calc(Object * W, const timing &tmg, bool limit = false);
        double func(string & f, double val);
        double func(string & f, double val1, double val2);
        double func(string &f, double cond, double val1, double val2);
        double oprt(string & f, double val1, double val2);
        double oprt(string & f, unsigned int i1, unsigned int i2, Object * W, const timing &tmg, bool limit=false);
        vector<string> funcs;
        string unit;
        string text;
        vector<string> opts;
        int lookup_operators(const string &s);
        int count_operators(const string &s);
        enum loc {self, source, destination, average_of_links};
        string ToString() const;
		vector<string> GetAllRequieredStartingBlockProperties();
		vector<string> GetAllRequieredEndingBlockProperties();
        Expression ReviseConstituent(const string &constituent_name, const string &quantity);
        bool RenameQuantity(const string &oldname, const string &newname);
        void ResetTermsSources();
        void ClearTermSources()
        {
            term_sources_determined = false;
            term_sources.clear();
        }
        bool SetQuanPointers(Object *W);
    protected:

    private:
        vector<double> term_vals;
        vector<bool> terms_calculated;
        vector<vector<int> > term_sources;
        bool term_sources_determined = false;
        vector<unsigned int> terms_source_counter;
        loc location = loc::self; //0: self, 1: start, 2: end

};


#endif // EXPRESSION_H
