#ifndef SOLUTIONLOGGER_H
#define SOLUTIONLOGGER_H
#include <fstream>
#include "Vector.h"
#include "Matrix.h"
#include "Matrix_arma.h"
#include "Vector_arma.h"

using namespace std;

class SolutionLogger
{
public:
    SolutionLogger();
    SolutionLogger(const string &filename);
    bool AssignFile(const string &filename);
    bool WriteString(const string &s);
    bool Close();
    bool WriteVector(const CVector &v);
    bool WriteVector(const CVector_arma &v);
    bool WriteMatrix(const CMatrix &v);
    bool WriteMatrix(const CMatrix_arma &v);
    void Flush();
private:
    ofstream file;
    vector<string> contents;
};

#endif // SOLUTIONLOGGER_H
