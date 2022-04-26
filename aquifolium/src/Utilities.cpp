#include "Utilities.h"

using namespace std;

namespace aquiutils
{

    int lookup(const vector<string> &s, const string &s1)
    {
        for (unsigned int i=0; i<s.size(); i++)
            if (s[i]==s1)
                return i;
        return -1;
    }

    int lookup(const vector<int> &s, const int &s1)
    {
        for (unsigned int i=0; i<s.size(); i++)
            if (s[i]==s1)
                return i;
        return -1;
    }

    int lookup(const vector<vector<int> > &s, const vector<int> &s1)
    {
        for (unsigned int i=0; i<s.size(); i++)
            if (s[i]==s1)
                return i;
        return -1;
    }

    int corresponding_parenthesis(string S, int i)
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

    int count(const string &s, const string &s1)
    {
        int out=0;
        for (unsigned int i=0; i<s.size()-s1.size()+1; i++)
            if (s.substr(i,s1.size())==s1)
                out++;
        return out;
    }

    bool parantheses_balance(string S)
    {
        if (count(S,"(") == count(S,")"))
            return true;
        else
            return false;
    }

    bool contains(const string &s, const string &s1)
    {
        for (unsigned int i=0; i<s.size()-s1.size()+1; i++)
            if (s.substr(i,s1.size())==s1)
                return true;
        return false;
    }

    string left(const string &s, int i)
    {
        return s.substr(0,i);
    }
    string right(const string &s, int i)
    {
        return s.substr(s.size()-i,i);
    }

    void remove(string &s,unsigned int i)
    {
        string S;
        for (unsigned int j=0; j<s.size(); j++)
            if (i!=j)
                S = S + s[j];
        s = S;
    }

    void replace(string &s,unsigned int i,string p)
    {
        string S;
        for (unsigned int j=0; j<s.size(); j++)
            if (i!=j)
                S = S + s[j];
            else
                S = S + p;
        s = S;
    }

    void remove(string &s,unsigned int pos, unsigned int len)
    {
        for (unsigned int i=pos; i<pos+len; i++)
            remove(s, pos);
    }


    bool remove(string &s, string s1)
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

    void insert(string &s, unsigned int pos, string s1)
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


    bool replace(string &s,string s1, string s2)
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




    void replace(string &s,unsigned int i, unsigned int j, string p)
    {
        string S;
        for (unsigned int k=0; k<s.size(); k++)
            if (k==i)
                S = S + p;
            else if (k<i || k>=i+j)
                S = S + s[j];

        s = S;
    }



    bool isnumber(char S)
    {
        if ((((int)S > 47) && ((int)S < 58)) || (S=='.') || (S=='-'))
            return true;
        else
            return false;
    }

    bool isnumber(string S)
    {
        bool res = true;
        for (unsigned int i = 0; i < S.size(); i++)
            if (!isnumber(S[i]))
                res = false;

        return res;
    }


    bool isintegernumber(string S)
    {
        bool out = true;
        for (unsigned int i = 0; i < S.size(); i++)
        {
            if (((int)S[i] <= 47) || ((int)S[i] >= 58))
                out = false;
        }
        return out;
    }

    double atof(const string &S)
    {
        return std::atof(S.c_str());
    }
    double atoi(const string &S)
    {
        return std::atoi(S.c_str());
    }

    string trim(const string &s)
    {
        if (s.find_first_not_of(' ') == string::npos) return "";

        return s.substr( s.find_first_not_of(' '), s.find_last_not_of(' ') + 1 );
    }

    void push_back(vector<string> &s, const vector<string> &s1)
    {
        for (unsigned int i=0; i<s1.size(); i++)
            s.push_back(s1[i]);
    }

    bool isnegative(const double &x)
    {
        if (x<-SMALLNUMBER)
            return true;
        else
            return false;
    }

    bool ispositive(const double &x)
    {
        if (x>SMALLNUMBER)
            return true;
        else
            return false;
    }

    bool iszero(const double &x)
    {
        if (fabs(x)<SMALLNUMBER)
            return true;
        else
            return false;
    }



    vector<string> split(const string &s, char del)
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

    vector<string> getline(ifstream& file)
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

    vector<string> getline(ifstream& file, char del1)
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

    vector<vector<string>> getline_op(ifstream& file,char del1)
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

    vector<string> split(const string &s, const vector<char> &del)
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

    vector<vector<string>> getline_op(ifstream& file,vector<char> del1)
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




    vector<vector<string>> getline_op_eqplus(ifstream& file)
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


    vector<string> split_curly_semicolon(string s)
    {
        vector<char> del2; del2.push_back('{'); del2.push_back('}'); del2.push_back(';');
        return split(s,del2);
    }

    vector<int> look_up(string s, char del)  //Returns a vector with indices of "del"
    {
        vector<int> out;
        for (unsigned int i=0; i<s.size(); i++)
            if (s[i]==del)
                out.push_back(i);

        return out;

    }

    vector<int> ATOI(vector<string> ii)
    {
        vector<int> res;
        for (unsigned int i=0; i<ii.size(); i++)
            res.push_back(atoi(ii[i].c_str()));

        return res;
    }

    vector<double> ATOF(vector<string> ii)
    {
        vector<double> res;
        for (unsigned int i=0; i<ii.size(); i++)
            res.push_back(atof(ii[i].c_str()));

        return res;
    }


    string tolower(const string &S)
    {
        string SS = S;
        for (unsigned int i=0; i<S.size(); i++)
        {
            SS[i] = std::tolower(S[i]);
        }
        return SS;
    }

    vector<string> tolower(const vector<string> &S)
    {
        vector<string> SS = S;
        for (unsigned int i = 0; i<S.size(); i++)
        {
            SS[i] = tolower(S[i]);
        }
        return SS;
    }

    void writeline(ofstream& f, vector<string> s, string del)
    {
        for (unsigned int i=0; i<s.size()-1; i++)
            f<<s[i]<<del;
        f<<s[s.size()-1]<<std::endl;
    }

    void writeline(ofstream& f, vector<vector<string>> s, string del, string del2)
    {
        for (unsigned int i=0; i<s.size()-1; i++)
        {	for (unsigned int j=0; j<s[i].size()-1; j++)
                f<<s[i][j]<<del2;
            f<<s[i][s[i].size()-1]<<del;
        }
        f<<s[s.size()-1][s[s.size()-1].size()-1]<<std::endl;
    }
    void writestring(ofstream& f, string s)
    {
        f<<s;
    }

    void writestring(string filename, string s)
    {
        ofstream file(filename);
        file << s + "\n";
        file.close();

    }
    void writenumber(ofstream& f, double s)
    {
        f<<s;
    }

    void writeendl(ofstream& f)
    {
        f<<std::endl;
    }

    double Heavyside(double x)
    {
        if (x>0) return 1; else return 0;
    }

    double Pos(double x)
    {
        if (x>0) return x; else return 0;
    }


    string numbertostring(const double &x, bool scientific)
    {
        string Result;          // string which will contain the result
        ostringstream convert;   // stream used for the conversion
        if (scientific)
            convert << std::scientific;
        convert << x;      // insert the textual representation of 'Number' in the characters in the stream
        Result = convert.str();
        return Result;
    }

    string numbertostring(const vector<double> &x, bool scientific)
    {
        string Result = "[";
        for (int i=0; i<x.size()-1;i++)
            Result += numbertostring(x[i],scientific)+",";
        Result += numbertostring(x[x.size()-1],scientific) + "]";
        return Result;
    }

    string numbertostring(int x)
    {
        string Result;          // string which will contain the result
        ostringstream convert;   // stream used for the conversion
        convert << x;      // insert the textual representation of 'Number' in the characters in the stream
        Result = convert.str();
        return Result;
    }

    string numbertostring(unsigned int x)
    {
        string Result;          // string which will contain the result
        ostringstream convert;   // stream used for the conversion
        convert << x;      // insert the textual representation of 'Number' in the characters in the stream
        Result = convert.str();
        return Result;
    }

    string numbertostring(const vector<int> &x, bool scientific)
    {
        string Result = "[";
        if (x.size()>0)
        {   for (int i=0; i<x.size()-1;i++)
               Result += numbertostring(x[i])+",";
            Result += numbertostring(x[x.size()-1]) + "]";
        }
        else
            Result += "]";
        return Result;
    }

    string tail(std::string const& source, size_t const length) {
        if (length >= source.size()) { return source; }
        return source.substr(source.size() - length);
    } // tail

    string tabs(int i)
    {
        string out;
        for (int j=0; j<i; j++)
            out += "\t";
        return out;
    }

    bool And(vector<bool> x) { bool out = true;  for (int i = 0; i < x.size(); i++) out &= x[i]; return out; }
    double max(vector<double> x) { double out = -1e+24;  for (int i = 0; i < x.size(); i++) out=std::max(out, x[i]); return out; }
    int max(vector<int> x)
    {	int out = -37000;
        for (int i = 0; i < x.size(); i++)
            out=std::max(out, x[i]);
        return out;

    }
    double Max(vector<double> x) { double out = -1e+24;  for (int i = 0; i < x.size(); i++) out=std::max(out, x[i]); return out; }
    int Max(vector<int> x)
    {	int out = -37000;
        for (int i = 0; i < x.size(); i++)
            out=std::max(out, x[i]);
        return out;

    }

    string remove_backslash_r(const string &ss)
    {
        string s = ss;
        if (!s.empty() && s[s.size() - 1] == '\r')
            s.erase(s.size() - 1);
        return s;

    }

    string GetOnlyFileName(const string &fullfilename)
    {
        vector<char> del;
        del.push_back('/');
        del.push_back('\\');
        vector<string> splittedbyslash = split(fullfilename,del);
        return splittedbyslash[splittedbyslash.size()-1];

    }

}


