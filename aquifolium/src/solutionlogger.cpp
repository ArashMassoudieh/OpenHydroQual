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
    if (file.is_open())
        file.close(); // Clean up old state
    file.open(filename, std::ofstream::out);
    return file.good();

}

bool SolutionLogger::WriteString(const string &s)
{
    contents.push_back(s);
    return true;
}

bool SolutionLogger::Close()
{
    if (file.is_open())
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
    if (!file.is_open()) return; // Prevent write to invalid stream
    for (const auto& line : contents)
        file << line << std::endl;
    contents.clear();
}
