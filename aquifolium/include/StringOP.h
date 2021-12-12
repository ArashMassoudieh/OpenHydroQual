#pragma once

#include <vector>
#include <string>
#include <sstream>

class CRxnNetwork;

using namespace std;

enum physical_params
{
	ks = 0, theta_s = 1, theta_r = 2, vg_alpha = 3, vg_n = 4, vg_m = 5, lambda = 6, storativity = 7, depression_storage = 12, storage_epsilon = 15, storage_n = 16, light = 18, temperature = 19, wind = 20, humidity = 21, vapor_pressure = 22, latent_heat_of_evaporation = 23, LAI = 24, K_LAI = 25, LAI_max = 26,
	plant_growth_rate_coefficient = 27, temperature_base = 28, temperature_spread_factor = 29, plant_biomass_decay_factor = 30, plant_leaf_decay_factor = 31, optimal_temperature = 32, pan_evaporation_rate = 33, maximum_bio_volume = 34
};

enum functions
{
	exp_ = 1, hsd_ = 2, min_ = 3, max_ = 4, lne_ = 5, lnt_ = 6, sgm_ = 7, pos_ = 8, sq1_ = 9, sqr_ = 10, frs_ = 11, fas_ = 12, ply_ = 13, mon_ = 14, sq2_ = 15, abs_ = 16, sqs_ = 17, mo1_ = 18, toy_ = 19, sig_=20};


class CStringOP
{
public:
	CStringOP(void);
	CStringOP(string S);
	CStringOP(string S, CRxnNetwork *R);
	~CStringOP(void);
    CStringOP& operator = (const CStringOP &P);
	CStringOP(const CStringOP &P);
	vector<CStringOP> terms;
	int nterms;
	int nopts;
	vector<int> operators;
    vector<string> breakparents(string S);
    vector<int> breakops(string S);
	bool physical_ch = false;
	bool constant = false;
	bool parameter = false;
	bool s_concentration = false;
	bool concentration = false;
	bool function = false;
	bool s_block = false;
	bool t_block = false;
	bool a_block = false;
	double value=0;
	int number=0;
	int phase=0;
	int particle_type=0;
	string expression;
};

int opertr(char a);
bool isnumber(char S);
vector<string> getline(ifstream&);
vector<string> getline(ifstream&, char del1);
vector<vector<string> > getline_op(ifstream&,char del1);
vector<vector<string> > getline_op(ifstream&,vector<char> del1);
vector<string> split(const string &s, char del=',');
vector<string> split(const string &s, const vector<char> &del);
vector<string> split_curly_semicolon(string s);
vector<vector<string> > getline_op_eqplus(ifstream&);
string trim(const string &s);
vector<int> ATOI(vector<string> ii);
vector<double> ATOF(vector<string> ii);
int getoperator(string S);
string tolower(const string &S);
vector<string> tolower(const vector<string> &S);
void writeline(ofstream&, vector<string>, string del);
void writeline(ofstream&, vector<vector<string> >, string del1, string del2);
void writestring(ofstream&, string s);
void writestring(string filename, string s);
void writenumber(ofstream&, double s);
void writeendl(ofstream&);
double Heavyside(double x);
double Pos(double x);
double pipe_poly(double x);
double mon(double x, double y);
vector<int> look_up(string s, char del);
string numbertostring(double x);
string numbertostring(int x);
string numbertostring(unsigned int x);
string numbertostring(vector<int> x);
string numbertostring(vector<double> x);
bool isintegernumber(string S);
int quan(string S);
string tail(string const& source, size_t const length); // tail
//template<class _Ty> _Ty min(_Ty _X, _Ty _Y);
//template<class _Ty> _Ty max(_Ty _X, _Ty _Y);
