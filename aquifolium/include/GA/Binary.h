
#pragma once
#include <vector>

using namespace std;

class CBinary  
{
public:
	CBinary();
	CBinary(int n);
    CBinary(int n, int preci);
	virtual ~CBinary();
    int nDigits = 4;
    vector<int> Digit;
	int precision;
	CBinary(const CBinary &B);
    CBinary operator = (const CBinary &B);
    CBinary operator + (const CBinary &B);
    CBinary extract(int spoint, int epoint);
    double decode(double minrange);
    int& operator[](unsigned int i);
    void show();
	bool sign;
    void mutate(double mu);
	
};

CBinary code(double x, double minrange, double maxrange, int precision);
void cross(CBinary &B1, CBinary &B2, int p);
void cross(CBinary &B1, CBinary &B2, vector<int> p);
void cross2p(CBinary &B1, CBinary &B2, int p1, int p2);

