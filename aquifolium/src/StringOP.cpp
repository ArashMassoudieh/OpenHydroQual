/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 * 
 * This file is part of OpenHydroQual.
 * 
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@cua.edu for details.
 */


#include "StringOP.h"
#include <string>
#include <iostream>
#include <fstream>
#include "math.h"
#include <algorithm>
#ifdef GIFMOD
#include "RxnNetwork.h"
#endif

using namespace std;

CStringOP::CStringOP(void)
{
	parameter = false;
	constant = false;
	concentration = false;
	s_concentration = false;
	physical_ch = false;
	function = false;
	s_block = false;
	t_block = false;
	a_block = false;
	s_concentration = false;
	nterms = 0;
	nopts = 0;
	phase = 0;
	particle_type = -2;
	number = -1;

}


CStringOP::~CStringOP(void)
{
}

CStringOP::CStringOP(string S)
{
	/*
	functions 1: exp
	          2: H
			  3: min
			  4: max
			  5:lne
			  6:lnt
			  7:lgs
	*/
	expression = S;
	constant = false;
	parameter = false;
	concentration = false;
	s_concentration = false;
	physical_ch = false;
	function = false;
	s_block = false;
	t_block = false;
	a_block = false;
	nterms = 0;
	nopts = 0;
	phase = 0;
	particle_type = -2;
	number = -1;

	int op_count = 0;
	int terms_count = 0;
	for (unsigned int i=0; i<S.size(); i++)
	{
		if ((i<S.size()) && (opertr(S[min(int(i),int(S.size()-1))]) < 6))
		{
			operators.push_back(opertr(S[i]));
			op_count++;
		}
		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='('))
		{
			int prntcnt = 1;
			for (unsigned int j=i+1; j<S.size(); j++)
			{
				if(S[j] == '(')
				{
					prntcnt++;
				}
				if(S[j] == ')')
				{
					prntcnt--;
					if (prntcnt == 0)
					{
						terms.push_back(CStringOP(S.substr(i+1,j-i-1)));
						terms_count++;
						i=j;
						j=S.size() + 1;

					}
				}
			}
		}

		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='_'))
		{
			int prntcnt = 0;
			string a = S.substr(i+1,4);
			int nn = 0;
			if (S.substr(i+1,3) == "exp")
				nn = exp_;
			if (S.substr(i+1,3) == "hsd")
				nn = hsd_;
			if (S.substr(i+1,3) == "min")
				nn = min_;
			if (S.substr(i+1,3) == "max")
				nn = max_;
			if (S.substr(i+1,3) == "lne")
				nn = lne_;
			if (S.substr(i+1,3) == "lnt")
				nn = lnt_;
			if (S.substr(i+1,3) == "sgm")
				nn = sgm_;
			if (S.substr(i+1,3) == "pos")
				nn = pos_;
			if (S.substr(i+1,3) == "sq1")
				nn = sq1_;
			if (S.substr(i+1,3) == "sqr")
				nn = sqr_;
			if (S.substr(i+1,3) == "frs")
				nn = frs_;
			if (S.substr(i+1,3) == "fas")
				nn = fas_;
			if (S.substr(i+1,3) == "ply")
				nn = ply_;
			if (S.substr(i+1,3) == "mon")
				nn = mon_;
			if (S.substr(i+1,3) == "sq2")
				nn = sq2_;
			if (S.substr(i+1,3) == "abs")
				nn = abs_;
			if (S.substr(i+1,3) == "sqs")
				nn = sqs_;
			if (S.substr(i + 1, 3) == "mo1")
				nn = mo1_;
			if (S.substr(i + 1, 3) == "sig")
				nn = sig_;
			if (S.substr(i + 1, 3) == "toy")
				nn = toy_;

			for (unsigned int j=i+4; j<S.size(); j++)
			{
				if(S[j] == '(')
				{
					prntcnt++;
				}
				if(S[j] == ')')
				{
					prntcnt--;
					if (prntcnt == 0)
					{
						terms.push_back(CStringOP(S.substr(i+5,j-i-5)));
						terms_count++;
						terms[terms.size()-1].function = 1;
						terms[terms.size()-1].number = nn;
						i=j;
						j=S.size() + 1;

					}
				}
			}
		}

		if ((i<S.size()) && (isnumber(S[min(int(i),(int)S.size()-1)]) == true))
		{
			unsigned int j=i+1;
			while ((j<S.size()) && (isnumber(S[min(int(j),(int)S.size()-1)]) == true)) j++;
			terms.push_back(CStringOP());
			terms[terms_count].value = atof(S.substr(i,j-i).c_str());
			terms_count++;
			terms[terms_count-1].constant = true;
			terms[terms_count-1].nterms = 1;
			i=j-1;
		}
		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='k'))
		{
			int j=i+1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (split(S.substr(i+2,j-i-2),':').size()>0) terms[terms_count].number = atoi(S.substr(i+2,j-i-2).c_str());
				terms_count++;
				terms[terms_count-1].parameter = true;
				terms[terms_count-1].nterms = 1;
				i = j;
			}
		}

		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='c'))
		{
			int j=i+1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (split(S.substr(i+2,j-i-2),':').size()>0) terms[terms_count].number = ATOI(split(S.substr(i+2,j-i-2),':'))[0];
				if (split(S.substr(i+2,S.size()-1),':').size()==2)
				{	terms[terms_count].phase = ATOI(split(S.substr(i+2,j-i-2),':'))[1];
					terms[terms_count].particle_type = -1;
				}
				else if (split(S.substr(i+2,S.size()-1),':').size()==1)
				{	terms[terms_count].phase = -1;
					terms[terms_count].particle_type = -1;
				}
				else if (split(S.substr(i+2,S.size()-1),':').size()==3)
				{
					terms[terms_count].phase = ATOI(split(S.substr(i+2,S.size()-1),':'))[2];
					terms[terms_count].particle_type = ATOI(split(S.substr(i+2,S.size()-1),':'))[1];
				}
				terms_count++;
				terms[terms_count-1].concentration = true;
				terms[terms_count-1].nterms = 1;
				i = j;
			}
		}

		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='g'))
		{
			int j=i+1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (split(S.substr(i+2,S.size()-1),':').size()==1)
				{	terms[terms_count].phase = ATOI(split(S.substr(i+2,S.size()-1),':'))[0];
					terms[terms_count].particle_type = -1;
				}
				if (split(S.substr(i+2,S.size()-1),':').size()==2)
				{
					terms[terms_count].phase = ATOI(split(S.substr(i+2,S.size()-1),':'))[1];
					terms[terms_count].particle_type = ATOI(split(S.substr(i+2,S.size()-1),':'))[0];
				}
				terms_count++;
				terms[terms_count-1].s_concentration = true;
				terms[terms_count-1].nterms = 1;
				i = j;
			}
		}

		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='s'))
		{
			int j=i+1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (isintegernumber(S.substr(i + 2, j-i-2)) == true)
					terms[terms_count].number = atoi(S.substr(i + 2, j-i-2).c_str());
				else
					terms[terms_count].number = quan(S.substr(i + 2, j-i-2));


				terms_count++;
				terms[terms_count-1].s_block = true;
				terms[terms_count-1].nterms = 1;
				i = j;
			}

		}

		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='e'))
		{
			int j=i+1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (isintegernumber(S.substr(i + 2, j-i-2)) == true)
					terms[terms_count].number = atoi(S.substr(i + 2, j - i - 2).c_str());
				else
					terms[terms_count].number = quan(S.substr(i + 2, j - i - 2));
				terms_count++;
				terms[terms_count-1].t_block = true;
				terms[terms_count-1].nterms = 1;
				i = j;
			}

		}

		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='a'))
		{
			int j=i+1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (isintegernumber(S.substr(i + 2, j-i-2)) == true)
					terms[terms_count].number = atoi(S.substr(i + 2, j-i-2).c_str());
				else
					terms[terms_count].number = quan(S.substr(i + 2, j-i-2));
				terms_count++;
				terms[terms_count-1].a_block = true;
				terms[terms_count-1].nterms = 1;
				i = j;
			}

		}



		if ((i<S.size()) && (S[min(int(i),(int)S.size()-1)]=='f'))
		{
			int j=i+1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());

				if (isintegernumber(S.substr(i + 2, j - i - 2)) == true)
					terms[terms_count].number = atoi(S.substr(i + 2, j - i - 2).c_str());
				else
					terms[terms_count].number = quan(S.substr(i + 2, j - i - 2));
				terms_count++;
				terms[terms_count-1].physical_ch = true;
				terms[terms_count-1].nterms = 1;
				i = j;
			}

		}
	}

	nterms = terms_count;
	nopts = op_count;

}


CStringOP::CStringOP(string S, CRxnNetwork *R)
{
#ifdef GIFMOD
	/*
	functions 1: exp
	2: H
	3: min
	4: max
	5:lne
	6:lnt
	7:lgs
	*/
	expression = S;
	constant = false;
	parameter = false;
	concentration = false;
	s_concentration = false;
	physical_ch = false;
	function = false;
	s_block = false;
	t_block = false;
	a_block = false;
	nterms = 0;
	nopts = 0;
	phase = 0;
	particle_type = -2;
	number = -1;

	int op_count = 0;
	int terms_count = 0;
	for (unsigned int i = 0; i<S.size(); i++)
	{
		if ((i<S.size()) && (opertr(S[min(int(i), int(S.size() - 1))]) < 6))
		{
			operators.push_back(opertr(S[i]));
			op_count++;
		}
		if ((i<S.size()) && (S[min(int(i), (int)S.size() - 1)] == '('))
		{
			int prntcnt = 1;
			for (unsigned int j = i + 1; j<S.size(); j++)
			{
				if (S[j] == '(')
				{
					prntcnt++;
				}
				if (S[j] == ')')
				{
					prntcnt--;
					if (prntcnt == 0)
					{
						terms.push_back(CStringOP(S.substr(i + 1, j - i - 1), R));
						terms_count++;
						i = j;
						j = S.size() + 1;

					}
				}
			}
		}

		if ((i<S.size()) && (S[min(int(i), (int)S.size() - 1)] == '_'))
		{
			int prntcnt = 0;
			string a = S.substr(i + 1, 4);
			int nn;
			if (S.substr(i + 1, 3) == "exp")
				nn = exp_;
			if (S.substr(i + 1, 3) == "hsd")
				nn = hsd_;
			if (S.substr(i + 1, 3) == "min")
				nn = min_;
			if (S.substr(i + 1, 3) == "max")
				nn = max_;
			if (S.substr(i + 1, 3) == "lne")
				nn = lne_;
			if (S.substr(i + 1, 3) == "lnt")
				nn = lnt_;
			if (S.substr(i + 1, 3) == "sgm")
				nn = sgm_;
			if (S.substr(i + 1, 3) == "pos")
				nn = pos_;
			if (S.substr(i + 1, 3) == "sq1")
				nn = sq1_;
			if (S.substr(i + 1, 3) == "sqr")
				nn = sqr_;
			if (S.substr(i + 1, 3) == "frs")
				nn = frs_;
			if (S.substr(i + 1, 3) == "fas")
				nn = fas_;
			if (S.substr(i + 1, 3) == "ply")
				nn = ply_;
			if (S.substr(i + 1, 3) == "mon")
				nn = mon_;
			if (S.substr(i + 1, 3) == "sq2")
				nn = sq2_;
			if (S.substr(i + 1, 3) == "abs")
				nn = abs_;
			if (S.substr(i + 1, 3) == "sqs")
				nn = sqs_;
			if (S.substr(i + 1, 3) == "mo1")
				nn = mo1_;
			if (S.substr(i + 1, 3) == "toy")
				nn = toy_;
			if (S.substr(i + 1, 3) == "sig")
				nn = sig_;

			for (unsigned int j = i + 4; j<S.size(); j++)
			{
				if (S[j] == '(')
				{
					prntcnt++;
				}
				if (S[j] == ')')
				{
					prntcnt--;
					if (prntcnt == 0)
					{
						terms.push_back(CStringOP(S.substr(i + 5, j - i - 5),R));
						terms_count++;
						terms[terms.size() - 1].function = 1;
						terms[terms.size() - 1].number = nn;
						i = j;
						j = S.size() + 1;

					}
				}
			}
		}

		if ((i<S.size()) && (isnumber(S[min(int(i), (int)S.size() - 1)]) == true))
		{
			unsigned int j = i + 1;
			while ((j<S.size()) && (isnumber(S[min(int(j), (int)S.size() - 1)]) == true)) j++;
			terms.push_back(CStringOP());
			terms[terms_count].value = atof(S.substr(i, j - i).c_str());
			terms_count++;
			terms[terms_count - 1].constant = true;
			terms[terms_count - 1].nterms = 1;
			i = j - 1;
		}
		if ((i<S.size()) && (S[min(int(i), int(S.size() - 1))] == 'k'))
		{
			int j = i + 1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (split(S.substr(i + 2, j - i - 2), ':').size()>0) terms[terms_count].number = R->look_up_rxn_parameters(S.substr(i + 2, j - i - 2).c_str());
				terms_count++;
				terms[terms_count - 1].parameter = true;
				terms[terms_count - 1].nterms = 1;
				i = j;
			}
		}

		if ((i<S.size()) && (S[min(int(i), (int)S.size() - 1)] == 'c'))
		{
			unsigned int j = i + 1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (split(S.substr(i + 2, j - i - 2), ':').size()>0)
				{
					terms[terms_count].number = R->look_up_constituent_no(split(S.substr(i + 2, j - i - 2), ':')[0]);

				}
				if (split(S.substr(i + 2, S.size() - 1), ':').size() == 2)
				{
					terms[terms_count].phase = ATOI(split(S.substr(i + 2, j - i - 2), ':'))[1];
					terms[terms_count].particle_type = -1;
				}
				else if (split(S.substr(i + 2, S.size() - 1), ':').size() == 1)
				{
					terms[terms_count].phase = -1;
					terms[terms_count].particle_type = -1;
				}
				else if (split(S.substr(i + 2, S.size() - 1), ':').size() == 3)
				{
					terms[terms_count].phase = ATOI(split(S.substr(i + 2, S.size() - 1), ':'))[2];
					terms[terms_count].particle_type = ATOI(split(S.substr(i + 2, S.size() - 1), ':'))[1];
				}
				terms_count++;
				terms[terms_count - 1].concentration = true;
				terms[terms_count - 1].nterms = 1;
				i = j;
			}
		}

		if ((i<S.size()) && (S[min(int(i), (int)S.size() - 1)] == 'g'))
		{
			unsigned int j = i + 1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (split(S.substr(i + 2, S.size() - 1), ':').size() == 1)
				{
					terms[terms_count].phase = ATOI(split(S.substr(i + 2, S.size() - 1), ':'))[0];
					terms[terms_count].particle_type = -1;
				}
				if (split(S.substr(i + 2, S.size() - 1), ':').size() == 2)
				{
					terms[terms_count].phase = ATOI(split(S.substr(i + 2, S.size() - 1), ':'))[1];
					terms[terms_count].particle_type = ATOI(split(S.substr(i + 2, S.size() - 1), ':'))[0];
				}
				terms_count++;
				terms[terms_count - 1].s_concentration = true;
				terms[terms_count - 1].nterms = 1;
				i = j;
			}
		}

		if ((i<S.size()) && (S[min(int(i), (int)S.size() - 1)] == 's'))
		{
			int j = i + 1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (isintegernumber(S.substr(i + 2, i - j - 2)) == true)
					terms[terms_count].number = atoi(S.substr(i + 2, i - j - 2).c_str());
				else
					terms[terms_count].number = quan(S.substr(i + 2, i - j - 2));


				terms_count++;
				terms[terms_count - 1].s_block = true;
				terms[terms_count - 1].nterms = 1;
				i = j;
			}

		}

		if ((i<S.size()) && (S[min(int(i), (int)S.size() - 1)] == 'e'))
		{
			int j = i + 1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (isintegernumber(S.substr(i + 2, i - j - 2)) == true)
					terms[terms_count].number = atoi(S.substr(i + 2, i - j - 2).c_str());
				else
					terms[terms_count].number = quan(S.substr(i + 2, i - j - 2));
				terms_count++;
				terms[terms_count - 1].t_block = true;
				terms[terms_count - 1].nterms = 1;
				i = j;
			}

		}

		if ((i<S.size()) && (S[min(int(i), (int)S.size() - 1)] == 'a'))
		{
			int j = i + 1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());
				if (isintegernumber(S.substr(i + 2, i - j - 2)) == true)
					terms[terms_count].number = atoi(S.substr(i + 2, i - j - 2).c_str());
				else
					terms[terms_count].number = quan(S.substr(i + 2, i - j - 2));
				terms_count++;
				terms[terms_count - 1].a_block = true;
				terms[terms_count - 1].nterms = 1;
				i = j;
			}

		}



		if ((i<S.size()) && (S[min(int(i), (int)S.size() - 1)] == 'f'))
		{
			int j = i + 1;
			while (S[j] != ']') j++;
			{
				terms.push_back(CStringOP());

				if (isintegernumber(S.substr(i + 2, j - i - 2)) == true)
					terms[terms_count].number = atoi(S.substr(i + 2, j - i - 2).c_str());
				else
					terms[terms_count].number = quan(S.substr(i + 2, j - i - 2));
				terms_count++;
				terms[terms_count - 1].physical_ch = true;
				terms[terms_count - 1].nterms = 1;
				i = j;
			}

		}
	}

	nterms = terms_count;
	nopts = op_count;
#endif
}

int opertr(char a)
{
	switch (a) {
		case '+':
			return 0;
		case '-':
			return 1;
		case '*':
			return 2;
		case '/':
			return 3;
		case '^':
			return 4;
		case ':':
			return 5;
	}
	return 999;
}

int getoperator(string S)
{
	int k=-1;
	for (unsigned int i=0; i<S.size(); i++)
		if (opertr(S[i])!=999)
			{	k=opertr(S[i]);
				break;}
	return k;

}

bool isnumber(char S)
{
	if ((((int)S > 47) && ((int)S < 58)) || (S=='.'))
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

int quan(string S)
{
	if (tolower(trim(S)) == "a") return 2;
	if (tolower(trim(S)) == "h") return 1;
	if (tolower(trim(S)) == "v") return 3;
	if (tolower(trim(S)) == "s") return 4;
	if (tolower(trim(S)) == "z0") return 5;
	if (tolower(trim(S)) == "se") return 9;
	if (tolower(trim(S)) == "theta") return 10;
	if (tolower(trim(S)) == "porosity") return theta_s+50;
	if (tolower(trim(S)) == "depression") return 12;
	if (tolower(trim(S)) == "vapor_diff") return 13;

	if (tolower(trim(S)) == "ks") return ks+50;
	if (tolower(trim(S)) == "theta_s") return theta_s+50;
	if (tolower(trim(S)) == "theta_r") return theta_r+50;
	if (tolower(trim(S)) == "vg_alpha") return vg_alpha+50;
	if (tolower(trim(S)) == "vg_n") return vg_n+50;
	if (tolower(trim(S)) == "vg_m") return vg_m+50;
	if (tolower(trim(S)) == "lambda") return lambda+50;
	if ((tolower(trim(S)) == "sc") || (tolower(trim(S)) == "storativity")) return storativity+50;
	if ((tolower(trim(S)) == "bulk_density") || (tolower(trim(S)) == "bd")) return 14;
	if (tolower(trim(S)) == "storage_epsilon") return storage_epsilon+50;
	if (tolower(trim(S)) == "storage_n") return storage_n+50;
	if (tolower(trim(S)) == "temperature") return temperature;
	if (tolower(trim(S)) == "light") return light;
	return -1;
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

string trim(const string &s)
{
	if (s.find_first_not_of(' ') == string::npos) return "";

	return s.substr( s.find_first_not_of(' '), s.find_last_not_of(' ') + 1 );
}

CStringOP& CStringOP::operator = (const CStringOP &P)
{
	concentration = P.concentration;
	physical_ch = P.physical_ch;
	s_concentration = P.s_concentration;
	constant = P.constant;
	nopts = P.nopts;
	nterms = P.nterms;
	function = P.function;
	number = P.number;
	operators = P.operators;
	parameter = P.parameter;
	terms = P.terms;
	value = P.value;
	s_block = P.s_block;
	t_block = P.t_block;
	a_block = P.a_block;
	phase = P.phase;
	expression = P.expression;
	particle_type = P.particle_type;
	return *this;
}

CStringOP::CStringOP(const CStringOP &P)
{
	concentration = P.concentration;
	s_concentration = P.s_concentration;
	physical_ch = P.physical_ch;
	constant = P.constant;
	function = P.function;
	nopts = P.nopts;
	nterms = P.nterms;
	number = P.number;
	operators = P.operators;
	parameter = P.parameter;
	terms = P.terms;
	value = P.value;
	s_block = P.s_block;
	t_block = P.t_block;
	a_block = P.a_block;
	expression = P.expression;
	phase = P.phase;
	particle_type = P.particle_type;
}

void make_a_folder_if_can(string path)
{



}

string tolower(const string &S)
{
	string SS = S;
	for (unsigned int i=0; i<S.size(); i++)
	{
		SS[i] = tolower(S[i]);
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

void writeline(ofstream& f, vector<string> s, string del=",")
{
	for (unsigned int i=0; i<s.size()-1; i++)
		f<<s[i]<<del;
	f<<s[s.size()-1]<<endl;
}

void writeline(ofstream& f, vector<vector<string>> s, string del=",", string del2="&")
{
	for (unsigned int i=0; i<s.size()-1; i++)
	{	for (unsigned int j=0; j<s[i].size()-1; j++)
			f<<s[i][j]<<del2;
		f<<s[i][s[i].size()-1]<<del;
	}
	f<<s[s.size()-1][s[s.size()-1].size()-1]<<endl;
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
	f<<endl;
}

double Heavyside(double x)
{
	if (x>0) return 1; else return 0;
}

double Pos(double x)
{
	if (x>0) return x; else return 0;
}

string numbertostring(double x)
{
	string Result;          // string which will contain the result
	ostringstream convert;   // stream used for the conversion
	convert << x;      // insert the textual representation of 'Number' in the characters in the stream
	Result = convert.str();
	return Result;
}

double pipe_poly(double x)
{
	double out;
	if (x < 0) out=0;
	else if (x >1) out=1;
	else out = -2.0255*pow(x,4)+1.9813*pow(x,3)+1.0318*pow(x,2)+0.0388*x;
	return out;
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


string numbertostring(vector<int> x)
{
	string xx;
	for (unsigned int i = 0; i < x.size(); i++)
	{
		xx = xx + numbertostring(x[i]);
		if (i < x.size() - 1) xx = xx + ", ";
	}
	return xx;
}

string numbertostring(vector<double> x)
{
	string xx;
	for (unsigned int i = 0; i < x.size(); i++)
	{
		xx = xx + numbertostring(x[i]);
		if (i < x.size() - 1) xx = xx + ", ";
	}
	return xx;
}

double mon(double x, double y)
{
	double d = 0.5*(x+fabs(x));
	return d/(d+y);
}


string tail(std::string const& source, size_t const length) {
	if (length >= source.size()) { return source; }
	return source.substr(source.size() - length);
} // tail

/*template<class _Ty> _Ty min(_Ty _X, _Ty _Y)
{
	if (_X > _Y) return _Y; else return _X;
}

template<class _Ty> _Ty max(_Ty _X, _Ty _Y)
{
	if (_X > _Y) return _X; else return _Y;
}*/
