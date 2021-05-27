#include "solutionlogger.h"

SolutionLogger::SolutionLogger()
{

}

SolutionLogger::SolutionLogger(const string &filename)
{
    AssignFile(filename);
}
bool SolutionLogger::AssignFile(const string &filename)
{
    file = ofstream(filename,std::ofstream::out);
    if (file.good())
        return true;
    else
        return false;
}

bool SolutionLogger::WriteString(const string &s)
{
    contents.push_back(s);
    return true;
}

bool SolutionLogger::Close()
{
    file.close();
    return true;
}

bool SolutionLogger::WriteVector(const CVector &v)
{
    contents.push_back(v.toString());
    return true;
}
bool SolutionLogger::WriteVector(const CVector_arma &v)
{
    contents.push_back(v.toString());
    return true;
}
bool SolutionLogger::WriteMatrix(const CMatrix &m)
{
    vector<string> out = m.toString();
    for (unsigned int i=0; i<out.size(); i++)
        contents.push_back(out[i]);
    return true;
}
bool SolutionLogger::WriteMatrix(const CMatrix_arma &m)
{
    vector<string> out = m.toString();
    for (unsigned int i=0; i<out.size(); i++)
        contents.push_back(out[i]);
    return true;
}
void SolutionLogger::Flush()
{
    for (unsigned int i=0; i<contents.size(); i++)
        file<<contents[i]<<endl;
    contents.clear();
}
