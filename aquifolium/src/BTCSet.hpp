#include "BTCSet.h"
#include "string.h"
#include <fstream>
#include "Utilities.h"
#include "DistributionNUnif.h"
#include <iostream>
#include "Vector.h"
#include "Matrix.h"
#include <omp.h>
#ifdef QT_version
#include "qdebug.h"
#include "qdatastream.h"
#endif // QT_version

using namespace std;

template <class T>
CTimeSeriesSet<T>::CTimeSeriesSet(void)
{
	nvars = 0;
	BTC.resize(nvars);
	unif = true;
}

template <class T> CTimeSeriesSet<T>::~CTimeSeriesSet(void)
{

}

template <class T>
CTimeSeriesSet<T>::CTimeSeriesSet(int n)
{
	nvars = n;
	BTC.resize(nvars);
	names.resize(nvars);
    for (int i=0; i<nvars; i++) BTC[i] = CTimeSeries<T>();
	unif = true;

}

template <class T>
CTimeSeriesSet<T>::CTimeSeriesSet(vector<vector<T>> &data, int writeInterval) :CTimeSeriesSet(data[0].size())
{
	for (unsigned int i = 0; i<data.size(); i++)
		if (i%writeInterval == 0) append(i, data[i]);
}

template <class T>
CTimeSeriesSet<T>::CTimeSeriesSet(int numberofBTCs, int sizeofBTCvector)
{
	nvars = numberofBTCs;
	BTC.resize(nvars);
	names.resize(nvars);
	for (int i = 0; i<nvars; i++)
        BTC[i] = CTimeSeries<T>(sizeofBTCvector);
	unif = true;
}

template <class T>
CTimeSeriesSet<T> merge(CTimeSeriesSet<T> A, CTimeSeriesSet<T> &B)
{
    CTimeSeriesSet<T> C = A;
	for (int i=0; i<B.nvars; i++)
	{	if (int(B.names.size())>i) C.names.push_back(B.names[i]);
		C.BTC.push_back(B.BTC[i]);
		C.nvars++;
	}
	return C;
}

template <class T>
CTimeSeriesSet<T> merge(vector<CTimeSeriesSet<T>> &A)
{
    CTimeSeriesSet<T> C = A[0];
	for (unsigned int i=1; i<A.size(); i++)
		C = merge(C,A[i]);
	return C;
}

template <class T>
void CTimeSeriesSet<T>::writetofile(char outputfile[])
{
	FILE *Fil;
	Fil = fopen(outputfile, "w");
    if (!Fil)
    {
        cout << "File '" + string(outputfile) +"' cannot be opened!"<<std::endl;
        return;
    }
    for (unsigned int i=0; i<names.size(); i++)
		fprintf(Fil , "t, %s, ", names[i].c_str());
	fprintf(Fil, "\n");
	for (int j=0; j<maxnumpoints(); j++)
	{
		for (int i=0; i<nvars; i++)
		{
            if (j<BTC[i].n)
                fprintf(Fil, "%lf, %le,", BTC[i].GetT(j), BTC[i].GetC(j));
			else
				fprintf(Fil, ", ,");

		}
		fprintf(Fil, "\n");
	}

	fclose(Fil);

}

template <class T>
void CTimeSeriesSet<T>::writetofile(string outputfile, bool writeColumnNameHeaders)
{
	FILE *Fil;
	Fil = fopen(outputfile.c_str() , "w");
	if (!Fil)
    {
        cout << "File '" + outputfile +"' cannot be opened!"<<std::endl;
        return;
    }
	if (writeColumnNameHeaders)
	{
		fprintf(Fil, "names, ");
		for (unsigned int i = 0; i < names.size(); i++)
			fprintf(Fil, "%s, ", names[i].c_str());
		fprintf(Fil, "\n");
	}
	fprintf(Fil , "//");
	for (unsigned int i=0; i<names.size(); i++)
		fprintf(Fil , "t, %s, ", names[i].c_str());
	fprintf(Fil, "\n");
	for (int j = 0; j<maxnumpoints(); j++)
	{
		for (int i=0; i<nvars; i++)
		{
			if (j<BTC[i].n)
                fprintf(Fil, "%lf, %le,", BTC[i].GetT(j), BTC[i].GetC(j));
			else
				fprintf(Fil, ", ,");

		}
		fprintf(Fil, "\n");
	}

	fclose(Fil);

}

template <class T>
void CTimeSeriesSet<T>::writetofile(string outputfile, int outputwriteinterval)
{
	FILE *Fil;
	Fil = fopen(outputfile.c_str() , "w");
    if (!Fil)
    {
        cout << "File '" + outputfile +"' cannot be opened!"<<std::endl;
        return;
    }
    for (unsigned int i=0; i<names.size(); i++)
		fprintf(Fil , "t, %s, ", names[i].c_str());
	fprintf(Fil, "\n");
	for (int j=0; j<maxnumpoints(); j++)
	{
		for (int i=0; i<nvars; i++)
		{
			if (j%outputwriteinterval==0)
			{
				if (j<BTC[i].n)
                    fprintf(Fil, "%lf, %le,", BTC[i].GetT(j), BTC[i].GetC(j));
				else
					fprintf(Fil, ", ,");
			}
		}
		if (j%outputwriteinterval==0)
			fprintf(Fil, "\n");
	}

	fclose(Fil);

}

template <class T>
int CTimeSeriesSet<T>::maxnumpoints()
{
	int m = 0;
	for (int i=0; i<nvars; i++)
		if (BTC[i].n>m) m = BTC[i].n;

	return m;
}

template <class T>
CTimeSeriesSet<T>::CTimeSeriesSet(const CTimeSeriesSet &B)
{
    BTC.clear();
    nvars = B.nvars;
	BTC.resize(nvars);
	names = B.names;
	BTC = B.BTC;
	unif = B.unif;
    filename = B.filename;
    name = B.name;

}

template <class T>
CTimeSeriesSet<T>::CTimeSeriesSet(const CTimeSeries<T> &B)
{
	nvars = 1;
	BTC.resize(1);
    filename = B.filename;
	BTC[0] = B;
}

template <class T>
CTimeSeriesSet<T>& CTimeSeriesSet<T>::operator = (const CTimeSeriesSet<T> &B)
{
    BTC.clear();
    nvars = B.nvars;
	BTC.resize(nvars);
	names = B.names;
    name = B.name;
    filename = B.filename;
	for (int i=0; i<nvars; i++)
		BTC[i] = B.BTC[i];
	unif = B.unif;
	return *this;

}

template <class T>
vector<T> CTimeSeriesSet<T>::interpolate(T t)
{
    vector<T> out;
	out.resize(nvars);
	for (int i=0; i<nvars; i++)
		out[i] = BTC[i].interpol(t);

	return out;
}

template <class T>
vector<T> CTimeSeriesSet<T>::interpolate(T t, int fnvars)
{
    vector<T> out;
	out.resize(fnvars);
	for (int i=0; i<min(nvars,fnvars); i++)
		out[i] = BTC[i].interpol(t);

	return out;
}

template <class T>
CTimeSeriesSet<T>::CTimeSeriesSet(string _filename, bool varytime)
{
	unif = false;
	vector<string> units;
    filename = _filename;
    ifstream file(filename);
	vector<string> s;
	nvars = 0;

	if (file.good() == false)
	{
		file_not_found = true;
		return;
	}
	if (varytime == false)
		while (file.eof() == false)
		{
			s = aquiutils::getline(file);
			if (s.size())
			{
				if (aquiutils::tail(s[0],5) == "names" || aquiutils::tail(s[0], 4) == "name")
					for (unsigned int i = 1; i < s.size(); i++) names.push_back(s[i]);
				if (aquiutils::tail(s[0],5) == "units" || aquiutils::tail(s[0], 4) == "unit")
					for (unsigned int i = 1; i < s.size(); i++) units.push_back(s[i]);
				if ((s[0].substr(0, 2) != "//") && (aquiutils::tail(s[0],5) != "names") && (aquiutils::tail(s[0],5) != "units"))
				{
					if (nvars == 0) { nvars = s.size() - 1; BTC.resize(nvars); }
					if (int(s.size()) == nvars + 1)
						for (int i = 0; i < nvars; i++)
						{
							BTC[i].t.push_back(atof(s[0].c_str()));
							BTC[i].C.push_back(atof(s[i + 1].c_str()));
							BTC[i].n++;
							if (BTC[i].t.size()>2)
                                if ((BTC[i].GetT(BTC[i].tSize() - 1) - BTC[i].GetT(BTC[i].tSize() - 2)) != (BTC[i].GetT(BTC[i].tSize() - 2) - BTC[i].GetT(BTC[i].tSize() - 3)))
									BTC[i].structured = false;

						}

				}
			}
		}
	else
		while (file.eof() == false)
		{
			s = aquiutils::getline(file);
			if (s.size() > 0)
			{
				if (aquiutils::tail(s[0],5) == "names" || aquiutils::tail(s[0], 4) == "name")
					for (unsigned int i = 1; i < s.size(); i++) if (aquiutils::trim(s[i])!="") names.push_back(s[i]);
				if (aquiutils::tail(s[0],5) == "units" || aquiutils::tail(s[0], 4) == "unit")
					for (unsigned int i = 1; i < s.size(); i++) units.push_back(s[i]);
				if ((s[0].substr(0, 2) != "//") && (aquiutils::tail(s[0],5) != "names") && (aquiutils::tail(s[0],5) != "units"))
				{
					if (nvars == 0) { nvars = s.size() / 2; BTC.resize(nvars); }

					for (int i = 0; i < nvars; i++)
					{
						if (int(s.size()) >= 2 * (i + 1))
							if ((aquiutils::trim(s[2 * i]) != "") && (aquiutils::trim(s[2 * i + 1]) != ""))
							{
								BTC[i].t.push_back(atof(s[2 * i].c_str()));
								BTC[i].C.push_back(atof(s[2 * i + 1].c_str()));
								BTC[i].n++;
								if (BTC[i].t.size()>2)
									if ((BTC[i].GetT(BTC[i].t.size() - 1) - BTC[i].GetT(BTC[i].t.size() - 2)) != (BTC[i].GetT(BTC[i].t.size() - 2) - BTC[i].GetT(BTC[i].t.size() - 3)))
										BTC[i].structured = false;
							}
					}
				}
			}
		}
	file.close();

	for (int i = 0; i < min(int(names.size()), nvars); i++)
		BTC[i].name = names[i];

	for (int i = 0; i < min(int(units.size()), nvars); i++)
		BTC[i].unit = units[i];

	//for (int i=0; i<nvars; i++)
	//	BTC[i].assign_D();

	if (int(names.size()) < nvars)
	{
		names.resize(nvars);
	}
	if (nvars == 1 && names[0] == "")
		names[0] = "Data";
	if (nvars > 1)
		for (int i = 0; i < nvars; i++)
			if (names[i] == "")
				names[i] = "Data (" + aquiutils::numbertostring(i) + ")";
}



template <class T>
void CTimeSeriesSet<T>::getfromfile(string _filename, bool varytime)
{
	unif = false;
	vector<string> units;
    ifstream file(_filename);
	vector<string> s;
	nvars = 0;
    filename = _filename;
	if (varytime==false)
		while (file.eof()== false)
		{
			s = aquiutils::getline(file);
			if (s.size()>0)
			{
				if (s[0] == "names")
					for (unsigned int i = 1; i < s.size(); i++) names.push_back(s[i]);
				if (s[0] == "units")
					for (unsigned int i = 1; i < s.size(); i++) units.push_back(s[i]);
				if ((s[0].substr(0, 2) != "//") && (s[0] != "names") && (s[0] != "units"))
				{
					if (nvars==0) {nvars = int(s.size()-1); BTC.resize(nvars); for (int i=0; i<nvars; i++) BTC[i].structured = true;}
					if (int(s.size())==nvars+1)
						for (int i=0; i<nvars; i++)
						{
							BTC[i].t.push_back(atof(s[0].c_str()));
							BTC[i].C.push_back(atof(s[i+1].c_str()));
							BTC[i].n++;
							if (BTC[i].n>2)
									if ((BTC[i].GetT(BTC[i].n-1)-BTC[i].GetT(BTC[i].n-2)) != (BTC[i].GetT(BTC[i].n-2)-BTC[i].GetT(BTC[i].n-3)))
										BTC[i].structured = false;
						}

				}
			}
		}
	else
		while (file.eof()== false)
		{
			s = aquiutils::getline(file);
			if (s.size() > 0)
			{
				if (s[0] == "names")
					for (unsigned int i = 1; i < s.size(); i++) names.push_back(s[i]);
				if (s[0] == "units")
					for (unsigned int i = 1; i < s.size(); i++) units.push_back(s[i]);
				if ((s[0].substr(0, 2) != "//") && (s[0] != "names") && (s[0] != "units"))
				{
					if (nvars == 0) { nvars = s.size() / 2; BTC.resize(nvars); for (int i = 0; i < nvars; i++) BTC[i].structured = true; }

					int n_line = s.size() / 2;
					for (int i = 0; i < n_line; i++)
					{
						if ((aquiutils::trim(s[2 * i]) != "") && (aquiutils::trim(s[2 * i + 1]) != ""))
						{
							BTC[i].t.push_back(atof(s[2 * i].c_str()));
							BTC[i].C.push_back(atof(s[2 * i + 1].c_str()));
							BTC[i].n++;
							if (BTC[i].n>2)
								if ((BTC[i].GetT(BTC[i].n - 1) - BTC[i].GetT(BTC[i].n - 2)) != (BTC[i].GetT(BTC[i].n - 2) - BTC[i].GetT(BTC[i].n - 3)))
									BTC[i].structured = false;
						}
					}
				}
			}
		}

	for (int i = 0; i < min(int(names.size()), nvars); i++)
		BTC[i].name = names[i];

	for (int i = 0; i < min(int(units.size()), nvars); i++)
		BTC[i].unit = units[i];

	file.close();
}

template <class T>
T CTimeSeriesSet<T>::maxtime()
{
	return BTC[0].GetT(BTC[0].n-1);

}

template <class T>
T CTimeSeriesSet<T>::mintime()
{
	return BTC[0].GetT(0);

}

template <class T>
T diff(CTimeSeriesSet<T> B1, CTimeSeriesSet<T> B2)
{
    T sum = 0;
	for (int i=0; i<B1.nvars; i++)
		sum += diff(B1.BTC[i],B2.BTC[i]);

	return sum;

}

template <class T>
CTimeSeriesSet<T> operator * (const CTimeSeriesSet<T> &BTC, const T &C)
{
    CTimeSeriesSet<T> A = BTC;
	A.BTC[0] = A.BTC[0]*C;
	return A;
}

template <class T>
vector<T> CTimeSeriesSet<T>::getrandom()
{
	int a = int(GetRndUniF(0,BTC[0].n));
    vector<T> res(nvars);
	for (int i=0; i<nvars; i++)
		res[i] = BTC[i].GetC(a);

	return res;
}

template <class T>
vector<T> CTimeSeriesSet<T>::getrandom(int burnin)
{
	int a = int(GetRndUniF(0,BTC[0].n-burnin));
    vector<T> res(nvars);
	for (int i=0; i<nvars; i++)
		res[i] = BTC[i].GetC(a+burnin);

	return res;
}

template <class T>
vector<T> CTimeSeriesSet<T>::getrow(int a)
{

    vector<T> res(nvars);
	for (int i = 0; i<nvars; i++)
		res[i] = BTC[i].GetC(a);

	return res;
}

template <class T>
vector<T> CTimeSeriesSet<T>::percentile(T x)
{
    vector<T> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].percentile(x));

	return v;
}

template <class T>
vector<T> CTimeSeriesSet<T>::mean(int limit)
{
    vector<T> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].mean(limit));
	return v;

}

template <class T>
vector<T> CTimeSeriesSet<T>::mean(int limit, vector<int> index)
{
    vector<T> v;
	for (unsigned int i = 0; i<index.size(); i++)
		v.push_back(BTC[index[i]].mean(limit));
	return v;

}

template <class T>
vector<T> CTimeSeriesSet<T>::std(int limit)
{
    vector<T> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].std(limit));
	return v;

}

template <class T>
vector<T> CTimeSeriesSet<T>::std(int limit, vector<int> index)
{
    vector<T> v;
	for (unsigned int i = 0; i<index.size(); i++)
		v.push_back(BTC[index[i]].std(limit));
	return v;

}

template <class T>
CMatrix CTimeSeriesSet<T>::correlation(int limit, int n)
{
	CMatrix r_xy(n);

	for (int i=0; i<n; i++)
		for (int j=0; j<=i; j++)
			r_xy[i][j] = R(BTC[i], BTC[j], limit);

	return r_xy;

}

template <class T>
vector<T> CTimeSeriesSet<T>::average()
{
    vector<T> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].average());
	return v;

}

template <class T>
vector<T> CTimeSeriesSet<T>::integrate()
{
    vector<T> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].integrate());
	return v;

}

template <class T>
vector<T> CTimeSeriesSet<T>::percentile(T x, int limit)
{
    vector<T> v;
	for (int i=0; i<nvars; i++)
		v.push_back(BTC[i].percentile(x,limit));

	return v;
}

template <class T>
vector<T> CTimeSeriesSet<T>::percentile(T x, int limit, vector<int> index)
{
	vector<double> v;
	for (unsigned int i = 0; i<index.size(); i++)
		v.push_back(BTC[index[i]].percentile(x, limit));

	return v;
}

template <class T>
CTimeSeriesSet<T> CTimeSeriesSet<T>::sort(int burnOut)
{
	CTimeSeriesSet r(nvars);
	if (burnOut < 0)
		burnOut = 0;
    vector<vector<T>> temp;
	temp.resize(nvars);
    vector<T> tempVec;

	int counter = 0;
	//clock_t tt0 = clock();

#pragma omp parallel

	#pragma omp for
	for (int i = 0; i < nvars; i++)
	{
		counter++;
		//qDebug() << "sorting BTC " << i << "(" << counter << ")";
		//clock_t t0 = clock();
//		r.BTC[i].C.resize(BTC[i].n - burnOut);
		tempVec.resize(BTC[i].n - burnOut);
		for (unsigned int j = 0; j < tempVec.size(); j++)
			tempVec[j] = BTC[i].GetC(j + burnOut);

		temp[i] = bubbleSort(tempVec);
//		r.BTC[i].C = QSort(temp);
		//clock_t t1 = clock() - t0;
		//float run_time = ((float)t1) / CLOCKS_PER_SEC;
		//qDebug() << "sorting BTC " << i << " finished in" << run_time << " sec (" << --counter << ")";
	}
	for (int i = 0; i < nvars; i++)
	{
		r.BTC[i].C.resize(BTC[i].n - burnOut);
		r.BTC[i].C = temp[i];
		r.BTC[i].n = temp[i].size();
	}
	//clock_t tt1 = clock() - tt0;
	//float run_time = ((float)tt1) / CLOCKS_PER_SEC;
	//qDebug() << "total time << " << run_time << "sec";

	return r;
}

template <class T>
CTimeSeriesSet<T> CTimeSeriesSet<T>::distribution(int n_bins, int n_columns, int limit)
{
	//qDebug() << "Distribution bins, columns, limit" << n_bins << n_columns << limit;
	CTimeSeriesSet A(n_columns);
	for (int i = 0; i < n_columns; i++)
	{
		A.BTC[i] = BTC[i].distribution(n_bins, limit);
		//qDebug() << "BTC[" << i << "] done";
	}

	return A;
}

template <class T>
CVector norm2dif(CTimeSeriesSet<T> &A, CTimeSeriesSet<T> &B)
{
	CVector res;
	for (int i=0; i<min(A.nvars,B.nvars); i++)
    {	CTimeSeries<T> BTC1 = A.BTC[i].Log(1e-5);
        CTimeSeries<T> BTC2 = B.BTC[i].Log(1e-5);
        res.append(diff_abs(BTC1,BTC2)/B.BTC[i].n);

    }

	return res;

}

template <class T>
void CTimeSeriesSet<T>::append(T t, vector<T> c)
{
	for (int i=0; i<min(int(c.size()), nvars); i++)
	{	BTC[i].structured = true;
		BTC[i].append(t,c[i]);
		if (BTC[i].n>2)
			if ((BTC[i].GetT(BTC[i].n-1)-BTC[i].GetT(BTC[i].n-2)) != (BTC[i].GetT(BTC[i].n-2)-BTC[i].GetT(BTC[i].n-3)))
				BTC[i].structured = false;
	}
}

template <class T>
CTimeSeries<T> CTimeSeriesSet<T>::add(vector<int> ii)
{
    CTimeSeries<T> A = BTC[ii[0]];
	A.structured = BTC[ii[0]].structured;
	for (unsigned int i=1; i<ii.size(); i++)
	if (unif==false)
	{	A+=BTC[ii[i]];
		A.structured = (A.structured && BTC[ii[i]].structured);
	}
	else
	{	A%=BTC[ii[i]];
		A.structured = (A.structured && BTC[ii[i]].structured);
	}

	return A;
}

template <class T>
CTimeSeries<T> CTimeSeriesSet<T>::add_mult(vector<int> ii, vector<T> mult)
{
    CTimeSeries<T> A;
	if (ii.size()>0)
	{	A = mult[0]*BTC[ii[0]];
		A.structured = BTC[ii[0]].structured;
		for (unsigned int i=1; i<ii.size(); i++)
		if (unif==false)
        {	CTimeSeries<T> BTC1 = mult[i]*BTC[ii[i]];
            A+=BTC1;
			A.structured = (A.structured && BTC[ii[i]].structured);
		}
		else
        {	CTimeSeries<T> BTC1 = mult[i]*BTC[ii[i]];
            A%=BTC1;
			A.structured = (A.structured && BTC[ii[i]].structured);
		}
	}
	else
	{
		A.setnumpoints(2);
		A.SetT(0,mintime());
		A.SetT(1,maxtime());
	}
	return A;
}

template <class T>
CTimeSeries<T> CTimeSeriesSet<T>::add_mult(vector<int> ii, CTimeSeriesSet &mult)
{
    CTimeSeries<T> A;
	if (ii.size()>0)
	{	A = mult.BTC[0]*BTC[ii[0]];
		A.structured = BTC[ii[0]].structured;
		for (unsigned int i=1; i<ii.size(); i++)
		if (unif==false)
        {	CTimeSeries<T> BTC1 = BTC[ii[i]]*mult.BTC[i];
            A+=BTC1;
			A.structured = (A.structured && BTC[ii[i]].structured && mult.BTC[i].structured);
		}
		else
        {	CTimeSeries<T> BTC1 = BTC[ii[i]]*mult.BTC[i];
            A%=BTC1;
			A.structured = (A.structured && BTC[ii[i]].structured && mult.BTC[i].structured);
		}
	}
	else
	{
		A.setnumpoints(2);
		A.SetT(0,mintime());
		A.SetT(1,maxtime());
	}
	return A;
}

template <class T>
CTimeSeries<T> CTimeSeriesSet<T>::divide(int ii, int jj)
{
    CTimeSeries<T> A;
	A.structured = (BTC[ii].structured && BTC[jj].structured);
	if (unif==false)
		A=BTC[ii]/BTC[jj];
	else
		A=BTC[ii]%BTC[jj];

	return A;


}

template <class T>
CTimeSeriesSet<T> CTimeSeriesSet<T>::make_uniform(T increment, bool assgn_d)
{
	if (nvars==0) return CTimeSeriesSet();
	CTimeSeriesSet out(nvars);
	out.names = names;

    for (int i = 0; i < nvars; i++)
        out.BTC[i].name = BTC[i].name;

	if (unif == true)
	{
		//qDebug() << "make uniform with unif option";
		for (int i = 0; i < nvars; i++)
		{

            out.BTC[i].append(BTC[i].GetT(0), BTC[i].GetC(0));
			if (assgn_d)
			{
				//qDebug() << "Assigning D to the original BTC";
				if (BTC[i].DSize() == 0) BTC[i].assign_D();
			}
		}
		for (int i=0; i<BTC[0].n-1; i++)
		{
			////qDebug() << i;
            int i1 = int((BTC[0].GetT(i)-BTC[0].GetT(0))/increment);
            int i2 = int((BTC[0].GetT(i+1)-BTC[0].GetT(0))/increment);
			for (int j=i1+1; j<=i2; j++)
			{
                T x = j*increment+BTC[0].GetT(0);
				for (int k=0; k<nvars; k++)
				{
                    T CC = (x-BTC[k].GetT(i))/(BTC[k].GetT(i+1)-BTC[k].GetT(i))*(BTC[k].GetC(i+1)-BTC[k].GetC(i))+BTC[k].GetC(i);

					out.BTC[k].append(x,CC);
					if (assgn_d)
					{
                        T DD = (x - BTC[k].GetT(i)) / (BTC[k].GetT(i + 1) - BTC[k].GetT(i))*(BTC[k].GetD(i + 1) - BTC[k].GetD(i)) + BTC[k].GetD(i);
						out.BTC[k].AppendD(DD);
					}
				}
			}
		}
	}
	else
	{
		//qDebug() << "make uniform without unif option";
		for (int k = 0; k < nvars; k++)
		{
			//qDebug() << "Variable:" + QString::fromStdString(names[k]);
			out.BTC[k] = BTC[k].make_uniform(increment);

		}
	}
	for (int k=0; k<nvars; k++)
    {   out.BTC[k].structured = true;
        out.BTC[k].name = BTC[k].name;
    }

	out.unif=true;

	return out;

}

template <class T>
CTimeSeriesSet<T> CTimeSeriesSet<T>::getpercentiles(vector<T> percents)
{
	CTimeSeriesSet X(1+percents.size());

	X.names.clear();

	X.setname(0, "Mean");
	for (unsigned int j=0; j<percents.size(); j++)
	{
		string Xname = to_string(percents[j]*100)+" %";
		X.setname(j + 1, Xname);
	}

    vector<T> XX(1+percents.size());
    vector<T> XX_prc(percents.size());

    T meanX;
	for (int i=0; i<BTC[0].n; i++)
	{
        vector<T> x;
		int count = 0;
		for (int j=0; j<nvars; j++)
			if (i<BTC[j].n)
            {	x.push_back(BTC[j].GetC(i));
				count++;
			}

		meanX = CVector(x).sum()/count;

		XX[0] = meanX;
		XX_prc = prcntl(x,percents);
		for (unsigned int j=0; j<percents.size(); j++)
			XX[j+1] = XX_prc[j];

        X.append(BTC[0].GetT(i),XX);
	}

	return X;
}

template <class T>
CVector CTimeSeriesSet<T>::out_of_limit(T limit)
{
	CVector v(nvars);
	for (int i=0; i<nvars; i++)
	{
        T n_tot = BTC[i].n;
        T n_exceed = 0;
		for (int j=0; j<BTC[i].n; j++)
        {		if (BTC[i].GetC(j) > limit)
				n_exceed++;		}

		v[i] = n_exceed/n_tot;
	}

	return v;
}

template <class T>
CTimeSeriesSet<T> CTimeSeriesSet<T>::add_noise(vector<T> std, bool logd)
{
	CTimeSeriesSet X(nvars);
	for (int i=0; i<nvars; i++)
		X.BTC[i] = BTC[i].add_noise(std[i],logd);

	return X;
}

template <class T>
CVector sum_interpolate(vector<CTimeSeriesSet<T>> &BTC, T t)
{
	if (BTC.size()==0) return CVector(1);
	CVector sum(max(max_n_vars(BTC),2)); //Be chacked later?
	for (unsigned int i=0; i<BTC.size(); i++)   //We can have several BTCs (Precipitation, Evaporation,...) and each one can have several variables (flow, concentration, ...)
	{
		for (int j=0; j<BTC[i].nvars; j++)
			sum[j]+=BTC[i].BTC[j].interpol(t);
	}
	return sum;
}

template <class T>
T sum_interpolate(vector<CTimeSeriesSet<T>> &BTC, T t, string name)
{
	if (BTC.size() == 0) return 0;
    T sum=0;
	for (unsigned int i = 0; i<BTC.size(); i++)
	{
		int ii = BTC[i].lookup(name);
		if (ii!=-1)
			sum += BTC[i].BTC[ii].interpol(t);
	}
	return sum;
}

template <class T>
void CTimeSeriesSet<T>::clear()
{
	BTC.clear();
    names.clear();
    nvars = 0;
}

template <class T>
vector<T> CTimeSeriesSet<T>::max_wiggle()
{
    T max_wig=0;
	int wiggle_id=-1;
	for (int i=0; i<nvars; i++)
    {	T a = BTC[i].wiggle();
		if (a>max_wig) wiggle_id = i;
		max_wig = max(max_wig,a);

	}
    vector<T> out;
	out.push_back(max_wig);
	out.push_back(wiggle_id);
	return out;
}

template <class T>
vector<T> CTimeSeriesSet<T>::max_wiggle_corr(int _n)
{
    T max_wig = 0;
	int wiggle_id = -1;
	for (int i = 0; i<nvars; i++)
	{
        T a = exp(-5*(1+BTC[i].wiggle_corr(_n)));
		if (a>max_wig) wiggle_id = i;
		max_wig = max(max_wig, a);

	}
    vector<T> out;
	out.push_back(max_wig);
	out.push_back(wiggle_id);
	return out;
}

template <class T>
vector<int> CTimeSeriesSet<T>::max_wiggle_sl(int ii, T tol)
{
    T max_wig = 0;
	int wiggle_id = -1;
	for (int i = 0; i<min(ii,nvars); i++)
	{
		int a = int(BTC[i].wiggle_sl(tol));
		if (a==1) wiggle_id = i;
		max_wig = max_wig || a;

	}
	vector<int> out;
	out.push_back(max_wig);
	out.push_back(wiggle_id);
	return out;
}

template <class T>
int max_n_vars(vector<CTimeSeriesSet<T>> &BTC)
{
	int k = 0;
	for (unsigned int i=0; i<BTC.size(); i++)
	{
		if (BTC[i].nvars>k) k = BTC[i].nvars;
	}
	return k;
}

template <class T>
void CTimeSeriesSet<T>::knockout(T t)
{
	for (int i=0; i<nvars; i++) BTC[i].knock_out(t);
}

template <class T>
int CTimeSeriesSet<T>::lookup(string S)
{
	int out = -1;
	for (unsigned int i = 0; i < names.size(); i++)
        if (S == names[i])
		{
			out = i;
			return out;
		}

	for (unsigned int i = 0; i < BTC.size(); i++)
        if (S == BTC[i].name)
		{
			out = i;
			return out;
		}


	return out;
}

template <class T>
CTimeSeries<T> &CTimeSeriesSet<T>::operator[](int index)
{
	while (int(names.size()) < index+1)
		names.push_back("");

	if (BTC[index].name == "")
		if (names[index] != "")
			BTC[index].name = names[index];
	if (names[index] == "")
		if (BTC[index].name != "")
			names[index] = BTC[index].name;
	return BTC[index];
}

template <class T>
CTimeSeries<T> &CTimeSeriesSet<T>::operator[](string BTCName)
{
	if (lookup(BTCName) != -1)
		return BTC[lookup(BTCName)];
	else
    {	CTimeSeries<T> BTC1 = CTimeSeries<T>();
        return BTC1;

    }
}

template <class T>
CTimeSeries<T> CTimeSeriesSet<T>::operator[](int index) const
{
    return BTC[index];
}

template <class T>
CTimeSeries<T> CTimeSeriesSet<T>::operator[](string BTCName) const
{
    if (lookup(BTCName) != -1)
        return BTC[lookup(BTCName)];
    else
    {	CTimeSeries<T> BTC1 = CTimeSeries<T>();
        return BTC1;

    }
}


template <class T>
bool CTimeSeriesSet<T>::Contains(string BTCName)
{
    if (lookup(BTCName) != -1)
        return true;
    else
        return false;
}

template <class T>
void CTimeSeriesSet<T>::pushBackName(string name)
{
	names.push_back(name);
	BTC[names.size() - 1].name = name;
}

template <class T>
void CTimeSeriesSet<T>::append(const CTimeSeries<T> &TS, string name)
{
    this->BTC.push_back(TS);
	if (name!="")
        BTC[BTC.size()-1].name = name;
	pushBackName(name);
	nvars = this->BTC.size();
}

template <class T>
void CTimeSeriesSet<T>::setname(int index, string name)
{
	while (names.size() < BTC.size())
		names.push_back("");

	names[index] = name;
	BTC[index].name = name;

}

template <class T>
void CTimeSeriesSet<T>::resize(unsigned int _size)
{
    for (unsigned int i = 0; i < BTC.size(); i++)
        BTC[i].resize(_size);
}

template <class T>
void CTimeSeriesSet<T>::ResizeIfNeeded(unsigned int _increment)
{

    for (unsigned int i = 0; i < BTC.size(); i++)
        BTC[i].ResizeIfNeeded(_increment);
}

template <class T>
void CTimeSeriesSet<T>::adjust_size()
{
    for (unsigned int i = 0; i < BTC.size(); i++)
        BTC[i].adjust_size();
}


#ifdef QT_version
template <class T>
void CTimeSeriesSet<T>::compact(QDataStream &data) const
{
	QMap<QString, QVariant> r;
	r.insert("nvars",nvars);
	QStringList namesList;
    for (unsigned int i = 0; i < names.size(); i++)
		namesList.append(QString::fromStdString(names[i]));
	r.insert("names", namesList);
	r.insert("unif",unif);
//omp_set_num_threads(16);
//#pragma omp parallel for
	data << r;
for (int i = 0; i < nvars; i++)
	{
//		QString code = QString("BTC %1").arg(i);
		//r.insert(code, BTC[i].compact());
		BTC[i].compact(data);
	}
	return;

}

template <class T>
CTimeSeriesSet CTimeSeriesSet<T>::unCompact(QDataStream &data)
{
	QMap<QString, QVariant> r;
	data >> r;
	CTimeSeriesSet c;
	c.nvars = r["nvars"].toInt();
	//c.BTC.resize(c.nvars);

	QStringList namesList = r["names"].toStringList();
	for (int i = 0; i < namesList.size(); i++)
		c.names.push_back(namesList[i].toStdString());

	c.unif = r["unif"].toBool();

//	omp_set_num_threads(16);
//#pragma omp parallel for
	for (int i = 0; i < c.nvars; i++)
	{
	//	QString code = QString("BTC %1").arg(i);
		c.BTC.push_back(CTimeSeries::unCompact(data));
	}

	return c;
}
#endif
